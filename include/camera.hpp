#pragma once

// io header
#include <iostream>
#include <string>
#include <fstream>

// thread header
#include <thread>
#include <mutex>                // 互斥锁
#include <condition_variable>   // 实现线程间的等待与通知

// opencv header
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

namespace stitching {

class Camera
{
public:
    Camera() = delete;
    Camera(string name, string path, int width=1280, int height=720, int fps=25, string fourcc="MJPG");
    ~Camera();
    void start(void);
    void stop(void);
    Mat get_frame(void);
    int get_frame_count(void) { return _frame_counter; };
    string get_name(void) { return _name; };

private:
    void _run(void);


    string _name;
    string _path;
    int _width;
    int _height;
    int _fps;
    int _fourcc;
    VideoCapture _cap;
    int _frame_counter = 0;
    Mat _frame;
    bool _running_flag = false;
    bool _frame_ready_flag = false;
    mutex _mtx_frame;
    condition_variable _cv_frame_ready;
    thread _thread_run;
};

}
