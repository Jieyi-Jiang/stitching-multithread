#include <iostream>

#include <thread>
#include <csignal>
#include <atomic>

#include <opencv2/opencv.hpp>

#include "camera.hpp"

using namespace std;
using namespace cv;
using namespace stitching;

atomic<bool> exit_flag(false);


void handle_signal(int signum)
{
    cout << "receive the siganal: " << signum << endl;
    exit_flag = true;
}

void setup_signal_handler()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
}

void display_frames(const vector<Camera*>& camera_list)
{
    // 在主线程中显示帧
    for (auto* camera : camera_list) {
        cv::namedWindow(camera->get_name(), cv::WINDOW_NORMAL);
        cv::resizeWindow(camera->get_name(), 800, 600);
        waitKey(100);
    }
    
    bool running = true;
    while (running) {
        for (auto* camera : camera_list) {
            cv::Mat frame = camera->get_frame();
            if (!frame.empty()) {
                cv::imshow(camera->get_name(), frame);
            }
        }

        // 统一的键盘检测
        int key = waitKey(1) & 0xFF;
        if (key == 'q' || exit_flag.load()) {  // 按'q'退出所有摄像头
            running = false;
            break;
        }
    }
    
    // 停止所有摄像头并销毁窗口
    // for (auto* camera : camera_list) {
    //     camera->stop();
    // }
    cv::destroyAllWindows();
}

int main(int argc, char* argv[])
{

    setup_signal_handler();
    Camera camera1("camera1", "/dev/video0", 1920, 1080, 25, "MJPG");
    Camera camera2("camera2", "/dev/video2", 1920, 1080, 25, "MJPG");
    Camera camera3("camera3", "/dev/video4", 1920, 1080, 25, "MJPG");
    camera1.start();
    camera2.start();
    camera3.start();
    vector<Camera*> camera_list({ &camera1, &camera2, &camera3});
    cout << "start display frames" << endl;
    display_frames(camera_list);
    cout << "stop display frames" << endl;
    // cv::namedWindow("camera1", cv::WINDOW_NORMAL);
    // cv::resizeWindow("camera1", 800, 600);
    // waitKey(10);
    // cv::namedWindow("camera2", cv::WINDOW_NORMAL);
    // cv::resizeWindow("camera2", 800, 600);
    // waitKey(10);
    // cv::namedWindow("camera3", cv::WINDOW_NORMAL);
    // cv::resizeWindow("camera3", 800, 600);
    // waitKey(10);
    cv::Mat frame;
    while (exit_flag == false)
    {
        // frame = camera1.get_frame();
        // if (!frame.empty())
        // {
        //     cv::imshow("camera1", frame);
        // }
        // frame = camera2.get_frame();
        // if (!frame.empty())
        // {
        //     cv::imshow("camera2", frame);
        // }
        // frame = camera3.get_frame();
        // if (!frame.empty())
        // {
        //     cv::imshow("camera3", frame);
        // }

        // int key = cv::waitKey(1) & 0xFF;
        // if (key == 'q' || exit_flag.load())
        // { // 按'q'退出所有摄像头
        //     exit_flag = true;
        //     break;
        // }
    }

    return 0;
}

