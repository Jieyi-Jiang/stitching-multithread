#include <iostream>

#include <thread>
#include <csignal>
#include <atomic>

#include <opencv2/opencv.hpp>

#include "Camera.hpp"

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
        int key = waitKey(10) & 0xFF;
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
    // vector<Camera*> camera_list({ &camera1, &camera2, &camera3});
    // vector<Camera*> camera_list({ &camera1 });

    cout << "start display frames" << endl;
    // display_frames(camera_list);
    cout << "stop display frames" << endl;
    cv::Mat frame;
    printf("waiting for exit signal...\n");
    // waitKey(0);
    while (exit_flag == false)
    {
        waitKey(1000);
    }
    cout << "exit signal received, stopping main process..." << endl;

    return 0;
}

