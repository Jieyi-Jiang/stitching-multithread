#include <iostream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"

#include "CameraParams.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "the number of arg is incorect" << endl;
    }

    stitching::CameraParams params = stitching::CameraParams(string(argv[1]));

    stitching::intrinsics_t intrinsics = params.get_intrinsics();
    cv::Mat Rot = params.get_rot();
    cv::Mat trans = params.get_trans();
    cout << Rot << endl;
    cout << trans << endl; 
    cout << "cx: " << intrinsics.cx << ", cy: " << intrinsics.cy << ", fx: " << intrinsics.fx \
         << " , fy: "  << intrinsics.fy << ", skew: " << intrinsics.skew << endl;
    cout << "distortion_coeffs: ";
    for (int i = 0; i < 5; ++ i)
    {
        cout << intrinsics.distortion_coeffs[i] << " ";
    }
    cout << endl;
}