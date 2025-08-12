#include "Est_seam.hpp"

void Est_seamm::update_img(vector<UMat>images_warped_f_in, vector<Point>corners_in, vector<UMat>masks_warped_in) {
    for (uint8_t i = 0;i < 2;++i) {
        myimg[i] = images_warped_f_in[i].clone();
        mymask[i] = masks_warped_in[i].clone();
    }
    corner = corners_in;
}

void Est_seamm::init_seamfinder() {
    mtx_esS.lock();
    Ptr<SeamFinder> seam_finder; //搜索器，函数内初始化(放到此处，减少搜索器生命周期)
    seam_finder = makePtr<detail::GraphCutSeamFinder>(detail::GraphCutSeamFinder::COST_COLOR_GRAD);
    //double t = getTickCount();
    //cout << myimg[0].size << endl << mymask[0].size<< endl << corner << endl;
    seam_finder->find(myimg, corner, mymask);
    maskflag_seam = false;
    for (uint8_t i = 0;i < 2;i++) {
        find_mask[i] = mymask[i].clone();
    }
    maskflag_seam = true;
    //std::cout << (getTickCount() - t) / getTickFrequency() * 1000<< endl;
    /*cv::imwrite("../out/warp_img1.jpg", myimg[0]);
    cv::imwrite("../out/warp_img2.jpg", myimg[1]);*/
    save_mask();
    bootflag_seam = true;
    id++;
    mtx_esS.unlock();
    cond.notify_all();
}

void Est_seamm::save_mask() {
    cv::imwrite("../out_mask/mask_left"+to_string(id)+".jpg", mymask[0]);
    cv::imwrite("../out_mask/mask_right"+to_string(id) + ".jpg", mymask[1]);
}