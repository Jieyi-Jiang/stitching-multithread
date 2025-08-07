//
// Created by jieyi on 25-8-7.
//

#ifndef MYSTITCHER_H
#define MYSTITCHER_H



#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui.hpp>

#include <opencv2/core/utility.hpp>

#include <opencv2/stitching/detail/matchers.hpp> //

using namespace std;
using namespace cv;
using namespace cv::detail;

class MyStitcher
{
private:
    // number of images to stitch
    int num = 3;
    // arrary of source input images
    vector<Mat> images_src;
    vector<Mat> images_down_sample;

    // feature extract
    Ptr<Feature2D> finder = SIFT::create();
    vector<ImageFeatures> features{vector<ImageFeatures>(num)};


};



#endif //MYSTITCHER_H
