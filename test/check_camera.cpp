#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
namespace fs = std::filesystem;

string fourccToString(int fourcc)
{
    char fourcc_str[5] = {0};
    fourcc_str[0] = fourcc & 0xFF;
    fourcc_str[1] = (fourcc >> 8) & 0xFF;
    fourcc_str[2] = (fourcc >> 16) & 0xFF;
    fourcc_str[3] = (fourcc >> 24) & 0xFF;
    return string(fourcc_str);
}



int main()
{
    vector<string> video_devices;
    // 枚举 /dev/video* 设备
    for (const auto &entry : fs::directory_iterator("/dev"))
    {
        string path = entry.path();
        if (path.find("/dev/video") != string::npos)
        {
            video_devices.push_back(path);
        }
    }

    cout << "检测到的视频设备：" << endl;
    for (auto &dev : video_devices)
    {
        cout << " - " << dev << endl;
    }

    cout << "\n开始检测每个摄像头的格式和推荐配置...\n" << endl;

    for (auto &dev : video_devices)
    {
        cv::VideoCapture cap(dev, cv::CAP_V4L2);  // 强制使用 V4L2
        if (!cap.isOpened())
        {
            cout << dev << " 打不开，可能被占用或不支持" << endl;
            continue;
        }

        cout << "===== 设备: " << dev << " =====" << endl;

        // 读取当前参数
        int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double fps = cap.get(cv::CAP_PROP_FPS);
        int fourcc = (int)cap.get(cv::CAP_PROP_FOURCC);

        cout << "当前默认分辨率: " << width << "x" << height << endl;
        cout << "当前默认FPS: " << fps << endl;
        cout << "当前FourCC: " << fourccToString(fourcc) << endl;

        // 尝试切换到 MJPG 并降低分辨率
        bool set_ok = cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        cap.set(cv::CAP_PROP_FPS, 25);

        int new_width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int new_height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double new_fps = cap.get(cv::CAP_PROP_FPS);
        int new_fourcc = (int)cap.get(cv::CAP_PROP_FOURCC);

        cout << "尝试设置 MJPG 1920x1080@30fps ..." << endl;
        cout << "实际生效参数: " << new_width << "x" << new_height
             << " @" << new_fps
             << " FourCC=" << fourccToString(new_fourcc) << endl;

        cv::Mat frame;
        if (cap.read(frame) && !frame.empty())
        {
            cout << "✅ 读取一帧成功，摄像头可用" << endl;
        }
        else
        {
            cout << "❌ 读取帧失败" << endl;
        }

        cap.release();
        cout << endl;
    }

    return 0;
}
