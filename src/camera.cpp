#include "camera.hpp"

namespace stitching {
    Camera::Camera(string name, string path, int width, int height, int fps, string fourcc)
    {
        _name = name;
        _path = path;
        _width = width;
        _height = height;
        _fps = fps;
        if (fourcc.length() != 4)
        {
            _fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
        }
        else
        {
            _fourcc = VideoWriter::fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
        }
        _cap = VideoCapture(_path, cv::CAP_V4L2);
        _cap.set(CAP_PROP_FRAME_WIDTH, _width);
        _cap.set(CAP_PROP_FRAME_HEIGHT, _height);
        _cap.set(CAP_PROP_FPS, _fps);
        _cap.set(CAP_PROP_FOURCC, _fourcc);
        cout << "Camera: " + _name + " open with path [" + _path + "]" << endl;
        cout << "Camera: " + _name + " open with width [" + to_string(_width) + "]" << endl;
        cout << "Camera: " + _name + " open with height [" + to_string(_height) + "]" << endl;
        cout << "Camera: " + _name + " open with fps [" + to_string(_fps) + "]" << endl;
        cout << "Camera: " + _name + " open with fourcc [" + to_string(_fourcc) + "]" << endl;
        if (_cap.isOpened() == false)
        {
            throw runtime_error("Camera: cannot open the [" + _name + "], with path [" + _path + "]");
        }
        _frame_counter = 0;
        _running_flag = false;
    }

    Camera::~Camera()
    {
        stop();
    }

    void Camera::start(void)
    {
        if (_running_flag == true)
        {
            cerr << "Camera: " + _name + " is running" << endl;
            return;
        }

        _running_flag = true;
        _thread_run = thread(&Camera::_run, this);

    }
    
    void Camera::stop(void)
    {
        if (_running_flag == false)
        {
            cerr << "Camera: " + _name + " has stopped" << endl;
        }

        _running_flag = false;
        _cv_frame_ready.notify_all();

        if ( _thread_run.joinable() )
        {
            _thread_run.join();
        }
        else
        {
            cerr << "Camera: " + _name + " running thread has joinned" << endl;
        }

        if ( _cap.isOpened() )
        {
            _cap.release();
        }
        else
        {

            cerr << "Camera: " + _name + " cap has released" << endl;
        }
    }


    void Camera::_run(void)
    {
        while (_running_flag && _cap.isOpened())
        {
            try
            {
                Mat temp_frame;
                bool ret = _cap.read(temp_frame);
                if (!ret || temp_frame.empty())
                {
                    // throw runtime_error("Camera: " + _name + " read frame failed");
                    std::cout << "Camera: " + _name + " read frame failed" << std::endl;
                    this_thread::sleep_for(chrono::milliseconds(1)); // 避免空转
                    continue;
                }
                {
                    // 加锁保护 _frame 资源
                    unique_lock<mutex> lock(_mtx_frame);
                    temp_frame.copyTo(_frame);
                    _frame_ready_flag = true;
                }
                _frame_counter++;
                // std::cout << "Camera: " + _name + " read frame " << _frame_counter << std::endl;
                // 通知等待帧的线程
                _cv_frame_ready.notify_one();

                // 如果需要控制帧率，可以这里加延时，例如 40ms 对应 25fps
                this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            catch(const std::exception& e)
            {
                std::cerr << "Camera: " + _name + ", exception: " << e.what() << std::endl;
                exit(1);
            }
        }
    }


    Mat Camera::get_frame(void)
    {
        unique_lock<mutex> lock(_mtx_frame);  // 自动加锁，离开作用域自动解锁
        _cv_frame_ready.wait_for(lock,  chrono::milliseconds(100), [this]{ return  _frame_ready_flag; });
        Mat ret_frame = _frame.clone();
        _frame_ready_flag = false;
        return ret_frame;
    }

}