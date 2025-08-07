#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui.hpp>

#include <opencv2/core/utility.hpp>

#include <opencv2/stitching/detail/matchers.hpp>

using namespace std;
using namespace cv;
using namespace cv::detail;

class MyStitcher
{
private:
    // arrary of source input images
    vector<Mat> images_src;

    // feature extract
    Ptr<Feature2D> finder = SIFT::create();
    // vector<ImageFeatures> features(3);

};

// // arrary of source input images
// vector<Mat> images_src;

// // feature extract
// Ptr<Feature2D> finder = SIFT::create();
// vector<ImageFeatures> features(3);
