#pragma once

#include <iostream>
#include <string>


#include "opencv2/opencv.hpp"

#include "yaml-cpp/yaml.h"

using namespace std;
using namespace YAML;

namespace stitching
{
struct intrinsics_t
{
    double fx;      // focal x
    double fy;      // focal y
    double cx;      // centre x
    double cy;      // centre y
    double skew;    // skew coefficient 
    vector<double> distortion_coeffs;// distortion coefficients
};



/**
 * @brief 相机参数类，用于存储和管理相机相关的各种参数
 */
class CameraParams
{
public:
    CameraParams() = delete;
    CameraParams(const string& yaml_path);
    intrinsics_t get_intrinsics() {return intrinsics; }
    cv::Mat get_rot() {return Rot;}
    cv::Mat get_trans() {return trans; }
    string get_path() {return path;}
    string get_name() {return name;}

private:
    string name;
    string path;
    int image_width;
    int image_height;
    double pixel_size_um;
    double sensor_width_um;
    double sensor_height_um;
    double lens_focal_mm;
    double FOV;
    intrinsics_t intrinsics;
    double calibration_error;
    double average_reprojection_error;
    cv::Mat Rot;
    cv::Mat trans;
};


}

