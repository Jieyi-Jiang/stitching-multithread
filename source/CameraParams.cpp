#include <fstream>
#include <iostream>
#include <string>

#include "opencv2/opencv.hpp"

#include "CameraParams.hpp"



namespace stitching
{
    CameraParams::CameraParams(const string& yaml_path)
    {
        YAML::Node config = YAML::LoadFile(yaml_path);
        name = config["name"].as<string>();
        path = config["path"].as<string>();
        image_width = config["image_width"].as<int>();
        image_height = config["image_height"].as<int>();
        pixel_size_um = config["pixel_size_um"].as<double>();
        sensor_width_um = config["sensor_width_um"].as<double>();
        sensor_height_um = config["sensor_height_um"].as<double>();
        lens_focal_mm = config["lens_focal_mm"].as<double>();
        FOV = config["FOV"].as<double>();
        intrinsics.fx = config["intrinsics"]["fx"].as<double>();
        intrinsics.fy = config["intrinsics"]["fy"].as<double>();
        intrinsics.cx = config["intrinsics"]["cx"].as<double>();
        intrinsics.cy = config["intrinsics"]["cy"].as<double>();
        intrinsics.skew = config["intrinsics"]["skew"].as<double>();
        intrinsics.distortion_coeffs = config["intrinsics"]["distortion_coeffs"].as<vector<double>>();
        if (intrinsics.distortion_coeffs.size() != 5)
        {
            throw runtime_error("The size of intrinsics.distortion_coeffs is error");
        }
        // Rot = cv::Mat(3, 3, CV_64F);
        // trans = cv::Mat(3, 1, CV_64F);
        vector<vector<double>> rot_vector = config["extrinsics"]["rot"].as<vector<vector<double>>>();
        Rot = cv::Mat(3, 3, CV_64F);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                Rot.at<double>(i, j) = rot_vector[i][j];
            }
        }
        vector<double> trans_vector = config["extrinsics"]["trans"].as<vector<double>>();
        trans = cv::Mat(3, 1, CV_64F);
        for (int i = 0; i < trans_vector.size(); ++i) {
            trans.at<double>(i) = trans_vector[i];
        }
    }
}