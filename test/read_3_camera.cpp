
#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

int main()
{
    cv::VideoCapture cap1("/dev/video0", cv::CAP_V4L2);  // 或者 "/dev/video0"
    if (!cap1.isOpened()) {
        std::cout << "Cannot open camera1!" << std::endl;
        return -1;
    }
    cv::VideoCapture cap2("/dev/video2", cv::CAP_V4L2);  // 或者 "/dev/video0"
    if (!cap2.isOpened()) {
        std::cout << "Cannot open camera2!" << std::endl;
        return -1;
    }
    cv::VideoCapture cap3("/dev/video4", cv::CAP_V4L2);  // 或者 "/dev/video0"
    if (!cap3.isOpened()) {
        std::cout << "Cannot open camera3!" << std::endl;
        return -1;
    }
    // cv::VideoCapture cap1("/dev/video0", cv::CAP_V4L2);
    // cv::VideoCapture cap2("/dev/video2", cv::CAP_V4L2);
    // cv::VideoCapture cap3("/dev/video4", cv::CAP_V4L2);

    // cap1.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    // cap2.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    // cap3.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap1.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    cap2.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    cap3.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));

    cap1.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap1.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap1.set(cv::CAP_PROP_FPS, 25);

    cap2.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap2.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap2.set(cv::CAP_PROP_FPS, 25);

    cap3.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap3.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap3.set(cv::CAP_PROP_FPS, 25);
    cv::Mat frame;
    while (true) 
    {
        cap1 >> frame;
        if (frame.empty()) 
        {
            std::cout << "Cannot receive frame from camera1!" << std::endl;
            // break;
        }
        else
        {
            cv::imshow("camera1", frame);
        }
        cap2 >> frame;
        if (frame.empty())
        {
            std::cout << "Cannot receive frame from camera2!" << std::endl;
            // break;
        }
        else
        {
            cv::imshow("camera2", frame);
        }
        cap3 >> frame;
        if (frame.empty())
        {
            std::cout << "Cannot receive frame from camera3!" << std::endl;
            // break;
        }
        else
        {
            cv::imshow("camera3", frame);
        }
        // cv::imshow("test", frame);
        if (cv::waitKey(1) == 27) break; // ESC退出
    }
}