#include <iostream>
#include <string>
#include <fstream>


#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

namespace stitching {

class Camera
{
public:
    Camera() = delete;     
    Camera(const string& name, const string& path);
    Camera(const string& name, const string& path, const Mat& camera_matrix, const Mat& dist_coeffs);
    void setCameraMatrix(const Mat& camera_matrix) { K = camera_matrix; }
    void setDistCoeffs(const Mat& dist_coeffs) { D = dist_coeffs; }
    cv::Mat& gCameraMatrix() { return K; }
    cv::Mat& gDistCoeffs() { return D; }
    cv::Mat& gFrameSource() { return frame_source; }
    cv::Mat& gFrameResized() { return frame_resized; }


private:
    std::string name;
    std::string path;
    cv::Mat K;
    cv::Mat D;
    cv::Mat frame_source;
    cv::Mat frame_resized;
    
};


}
