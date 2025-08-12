//
// Created by jieyi on 25-8-7.
//

#include "MyStitcher.hpp"
#include "yaml-cpp/yaml.h"

namespace stitching
{

MyStitcher::MyStitcher(const int img_num, const string prama_file)
{
    num = img_num;
    vector<Mat> images_src = vector<Mat>(num);
    vector<Mat> images_down_sample = vector<Mat>(num);
    vector<UMat> images_warped = vector<UMat>(num);
    vector<UMat> images_warped_f = vector<UMat>(num);
    mp_down_sample = 0.6;

    // finder = SIFT::create();
    finder = xfeatures2d::SURF::create(); 
    features = vector<ImageFeatures>(num);

    conf_thresh = 1.0;        // Threshold for two images are from the same panorama confidence.
    try_use_gpu = false;       // Wether try to use the gpu to accelerate the computing
    match_conf = 0.3f;        // Confidence threshold or match distances ration threshold, the lower the value the more restrict 
    num_matches_thresh1 = 6;    // Min number of matches required for 2D proj TF, estimate used in the inliners clssification step
    num_mathces_thresh2 = 6;    // Re-estimate used in the inliners clssification step
    matches_confidence_thresh = 3.0; // Matching confidence threshold, the higher the the more restrict

    matcher =  makePtr<BestOf2NearestMatcher>(
        try_use_gpu,                 
        match_conf,                 
        num_matches_thresh1,        
        num_mathces_thresh2,       
        matches_confidence_thresh    
    );  

    estimator = makePtr<HomographyBasedEstimator>();
    adjuster = makePtr<detail::BundleAdjusterReproj>();
    ba_refine_mask = "xxxxx";
    refine_mask = Mat::zeros(3, 3, CV_8U);
    warper_creator = makePtr<cv::SphericalWarper>();
    compensator = ExposureCompensator::createDefault(ExposureCompensator::GAIN_BLOCKS);
    seam_finder = makePtr<detail::GraphCutSeamFinder>(GraphCutSeamFinderBase::COST_COLOR);
}

void MyStitcher::load_images(const vector<Mat> &images) 
{
    if (images.size() < num)
    {
        throw runtime_error("Not enough images");
    }
    for (int i = 0; i < num; ++ i)
    {
        images_src[i] = images[i].clone();
    }
}

void MyStitcher::cal_feature()
{
    
}

}