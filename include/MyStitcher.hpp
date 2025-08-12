//
// Created by jieyi on 25-8-7.
//

#ifndef MYSTITCHER_H
#define MYSTITCHER_H



#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui.hpp"

#include "opencv2/core/utility.hpp"

// ImageFeatures, MatchesInfo, FeaturesMatcher
#include "opencv2/stitching/detail/matchers.hpp" 
// SphericalWarper
#include "opencv2/stitching/detail/warpers.hpp"

#ifdef HAVE_OPENCV_XFEATURES2D // 扩展模块，包含专利保护的部分
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#endif
// feature2d - 默认库，开源，可免费商用，实时性优先
// SITF, ORB, BRISK AKAZE, KAZE, FAST, AGAST, GFTT, Harris, SimpleBlobDetector, MSER
// xfeature2d - 扩展库，里面有部分算法是收专利保护的，精度优先
// SURF,

using namespace std;
using namespace cv;
using namespace cv::detail;

namespace stitching
{

class MyStitcher
{
public:
    MyStitcher() = default;
    ~MyStitcher() = default;
    MyStitcher(const int num, const string prama_file);
    void load_images(const vector<Mat> &images);
    void load_parameters();
    void cal_feature();
    void match_features();
    void cal_homography();
    void warp_images();
    void stitch_images();
    void save_images(string file_name);
private:
    ///////////////////////////// Basic Information ////////////////////////////
    // Number of images to stitch
    int num = 3;
    // Path of parameters file 
    string prama_file_path = "";

    ///////////////////////////// Arrary of images /////////////////////////////
    vector<Mat> images_src;
    vector<Mat> images_down_sample;
    vector<UMat> images_warped;
    vector<UMat> images_warped_f;
    int mp_down_sample = 0.6;
    vector<CameraParams> cameras; // camera parameters

    ///////////////////////////// Feature extract //////////////////////////////
    Ptr<Feature2D> finder; // high precision but slower
    // Ptr<Feature2D> finder = features2d::SIFT::create();  // faster
    vector<ImageFeatures> features;
    
    ///////////////////////////////// Matching /////////////////////////////////
    // The feature matching is nescessary for calculating the homography matrix
    // Or we can calibrating the camera parameters and homography matrix privously
    vector<MatchesInfo> pairwise_matches;
    float conf_thresh = 1.0;        // Threshold for two images are from the same panorama confidence.
    bool try_use_gpu = false;       // Wether try to use the gpu to accelerate the computing
    float match_conf = 0.3f;        // Confidence threshold or match distances ration threshold, the lower the value the more restrict 
    int num_matches_thresh1 = 6;    // Min number of matches required for 2D proj TF, estimate used in the inliners clssification step
    int num_mathces_thresh2 = 6;    // Re-estimate used in the inliners clssification step
    double matches_confidence_thresh = 3.0; // Matching confidence threshold, the higher the the more restrict
    Ptr<FeaturesMatcher> matcher;
    
    ///////////////////// Estimating the Homography Matrix /////////////////////
    // Ptr<Estimator> estimator = makePtr<AffineBasedEstimator>();
    Ptr<Estimator> estimator;
    Ptr<detail::BundleAdjusterBase> adjuster;
    string ba_refine_mask = "xxxxx";
    Mat_<uchar> refine_mask;
    
    ///////////////////////////////// Warping //////////////////////////////////
    Ptr<WarperCreator> warper_creator;
    Ptr<RotationWarper> warper;

    /////////////////////////// Exposure Compensation //////////////////////////
    Ptr<ExposureCompensator> compensator;

    /////////////////////////////// Seam Blending //////////////////////////////
    Ptr<SeamFinder> seam_finder;

};


}

#endif //MYSTITCHER_H
