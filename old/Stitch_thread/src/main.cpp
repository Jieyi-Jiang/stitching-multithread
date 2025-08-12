#include "Camera_input.hpp"
#include "KeyPoint.hpp"
#include "Est_exposure.hpp"
#include "Est_seam.hpp"
#include "Stitch_frame.hpp"
#include <queue>
#include <thread>
#include <filesystem>
#include <atomic>
#include <chrono>
#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;
using namespace std;

#define CAMERA_NUM 2
#define LOGLN(msg) std::cout << msg << std::endl
#define TIME_LOG 1
#define ORG_EXPOSE 0
#define IsThread 0
#define videoHave 1

bool exit_flag = false; //ȫ���˳���־������Ƶ����ȡΪ�գ�

Video_Capture img_queue1;
Video_Capture img_queue2;
Find_Feature feature1;
Find_Feature feature2;
Match_Features feature_match;
Est_exposure est_expose;
Est_seamm est_seam;
graph_blender blend_item;

///////////////////////////////////////thread1////////////////////////////////////////////////////
void video_input1(string path) {
    VideoCapture cap(path);
    if (!cap.isOpened()) {
        LOGLN("�޷�����Ƶ�ļ�!");
        return;
    }
    //params
    Mat frame;
    int  wrap_flag;
    float wrap_scale;
    cv::detail::CameraParams cam_param;
    double t;
    int i = 0;
    while (cap.read(frame)) {
        if (exit_flag) { break; }
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        t = getTickCount();
        //get org_img
        if (frame.empty()) continue;
        img_queue1.org_push(frame);
        //warp
        unique_lock<mutex> lk_cameras(feature_match.mtx_cameras);
        wrap_flag = feature_match.boot_flag;
        if (wrap_flag > 0) {
            wrap_scale = feature_match.warped_image_scale;
            feature_match.copy_camera(cam_param, feature_match.cameras[0]);
        }
        lk_cameras.unlock();
        if (wrap_flag > 0) {
            img_queue1.warp(frame, cam_param, wrap_scale);
#if 0
            Mat warp_img;
            img_queue1.rwc_pop(warp_img);
            warp_img.convertTo(warp_img, CV_8U);
            img_queue1.save_img(warp_img);
#endif
        }

#if TIME_LOG
        LOGLN("video_input1 : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif
    }

    exit_flag = true;
    img_queue1.signal_exit();
    LOGLN("----thread video 1 ---- �˳�" << "total:" << i);
}
/////////////////////////////////////// thread2 ////////////////////////////////////////////////////
void video_input2(string path) {
    VideoCapture cap(path);
    if (!cap.isOpened()) {
        LOGLN("�޷�����Ƶ�ļ�!");
        return;
    }
    double t;
    Mat frame;
    bool wrap_flag;
    float wrap_scale;
    cv::detail::CameraParams cam_param;

    while (cap.read(frame)) {
        if (exit_flag) { break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        t = getTickCount();
        if (frame.empty()) continue;
        img_queue2.org_push(frame);
        //warp
        unique_lock<mutex> lk_cameras(feature_match.mtx_cameras);
        wrap_flag = feature_match.boot_flag;
        if (feature_match.boot_flag) {
            wrap_scale = feature_match.warped_image_scale;
            feature_match.copy_camera(cam_param, feature_match.cameras[1]);
        }
        lk_cameras.unlock();
        if (wrap_flag) {
            img_queue2.warp(frame, cam_param, wrap_scale);
#if 0
            Mat warp_img;
            img_queue2.rwc_pop(warp_img);
            warp_img.convertTo(warp_img, CV_8U);
            img_queue2.save_img(warp_img);
#endif
        }

#if TIME_LOG
        LOGLN("video_input2 : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif
    }

    exit_flag = true;
    img_queue2.signal_exit();
    LOGLN("----thread video 2 ---- �˳�");
}
/////////////////////////////////////// thread3 ////////////////////////////////////////////////////
void find_feature_1() {
    Mat dst;
    double t;
    while (true) {
        if (exit_flag) { break; }
        if (img_queue1.org_pop(dst)) {
            t = getTickCount();
            feature1.find(dst);
#if TIME_LOG
            LOGLN("find_feature1 : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif
        }
        else {
            LOGLN("----thread feature 1 ---- �˳�");
            break;
        }
    }
}
/////////////////////////////////////// thread4 ////////////////////////////////////////////////////
void find_feature_2() {
    Mat dst;
    double t;
    while (true) {
        if (exit_flag) { break; }
        if (img_queue2.org_pop(dst)) {
            t = getTickCount();
            feature2.find(dst);

#if TIME_LOG          
            LOGLN("find_feature2 : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif
        }
        else {
            LOGLN("----thread feature 2 ---- �˳�");
            break;
        }
    }
}
/////////////////////////////////////// thread5 ////////////////////////////////////////////////////
void match_feature() {
    vector<detail::ImageFeatures> features;
    vector<Mat> imgs;
    imgs.resize(CAMERA_NUM);
    features.resize(CAMERA_NUM);
    double t;
    LOGLN("�ȴ���������ȡ");
    unique_lock<mutex> lk_f1(feature1.mtx_feature);
    while (!feature1.boot_flag) {
        feature1.cond.wait(lk_f1);
    }
    lk_f1.unlock();
    unique_lock<mutex> lk_f2(feature2.mtx_feature);
    while (!feature2.boot_flag) {
        feature2.cond.wait(lk_f1);
    }
    lk_f2.unlock();
    LOGLN("��ʼƥ��");
    while (1) {
        if (exit_flag) { break; }
        t = getTickCount();
        //copy
        feature1.mtx_feature.lock();
        feature2.mtx_feature.lock();
        //LOGLN("��ȡ");
        imgs[0] = feature1.get_dst();
        imgs[1] = feature2.get_dst();
        features[0] = feature1.get_feature();
        features[1] = feature2.get_feature();
        feature1.mtx_feature.unlock();
        feature2.mtx_feature.unlock();
        //process
        feature_match.update_features_imgs(features, imgs);
        //LOGLN("ƥ��");
        feature_match.match();
        //LOGLN("ƥ����ɣ���ʼ�����������");
        feature_match.est_params();
#if TIME_LOG   
        LOGLN("match_feature : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif 

    }
    LOGLN("----thread match---- �˳�");
}
/////////////////////////////////////// thread6 ////////////////////////////////////////////////////
void stitch_frame() {
    // update -----------------------------------
    blend_union bu;
    // end --------------------------------------
    vector<Mat>images_blend_s{ Mat(),Mat() };
    vector<Mat>masks_blend_s{ Mat(),Mat() };
    LOGLN("�ȴ�������ƥ��");
    unique_lock<mutex> lk_cameras(feature_match.mtx_cameras);
    while (!feature_match.boot_flag) {
        feature_match.cond.wait(lk_cameras);
    }
    lk_cameras.unlock();
    LOGLN("�ȴ��ع�");
    unique_lock<mutex> lk_exposure(est_expose.mtx_esE);
    while (!est_expose.bootflag_exposure_done) {
        est_expose.cond.wait(lk_exposure);
    }
    lk_exposure.unlock();
    LOGLN("��ʼƴ��");
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));   
    auto totol_start = chrono::high_resolution_clock::now();
    while (1) {
        if (exit_flag) { break; }
        if (!est_expose.corners_done_flag || !est_expose.graph_done_flag) { continue; }
        auto start = chrono::high_resolution_clock::now();
        /*
        // update ------------------------------------------
        images_blend_s[0] = bu.blend_img1.clone();
        images_blend_s[1] = bu.blend_img2.clone();
        masks_blend_s[0] = bu.blend_mask1.clone();
        masks_blend_s[1] = bu.blend_mask2.clone();
        // end ----------------------------------------------
        */
        est_expose.mtx_esE.lock();
        for (uint8_t i = 0;i < CAMERA_NUM;i++) {
            images_blend_s[i] = est_expose.use_imgs[i].clone();
            masks_blend_s[i] = est_expose.use_masks[i].clone();
        }
        blend_item.update_source(images_blend_s, masks_blend_s, est_expose.corners, est_expose.sizes);
        est_expose.mtx_esE.unlock();
        blend_item.graph_blend();

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        LOGLN("ƴ�Ӵ���ʱ��: " << duration.count() << " ms");
    }
    auto total_end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(total_end - totol_start);
    double average_time = static_cast<double>(duration.count()) / blend_item.id;
    LOGLN("ƴ��ƽ��ʱ��: " << average_time << " ms");
    LOGLN("----thread blend---- �˳�");
}
/////////////////////////////////////// thread7 ////////////////////////////////////////////////////
void Est_exp() {
    // update ------------------------------------
    rwc_org ro1, ro2;
    // end ---------------------------------------
    Mat dst1, dst2;
    vector<Point>corners;
    vector<Mat>warped_masks;
    vector<Mat>images_warped;
    vector<detail::CameraParams>cams = {
        detail::CameraParams(),
        detail::CameraParams()
    };
    float workscale;
    float warped_img_scale;
    est_expose.init_all();
    double t;
    LOGLN("�ȴ��������");
    unique_lock<mutex> lk_cameras(feature_match.mtx_cameras);
    while (!feature_match.boot_flag) {
        feature_match.cond.wait(lk_cameras);
    }
    lk_cameras.unlock();
    LOGLN("���������ȡ����ʼ���ع���");
#if ORG_EXPOSE
    img_queue1.org_pop(dst1);
    img_queue2.org_pop(dst2);
    feature1.mtx_feature.lock();
    workscale = feature1.work_scale;
    feature1.mtx_feature.unlock();
    img_queue1.mtx_corner.lock();
    warped_masks.push_back(img_queue1.mask_warped.clone());
    corners.push_back(img_queue1.corners);
    img_queue1.mtx_corner.unlock();
    img_queue2.mtx_corner.lock();
    warped_masks.push_back(img_queue2.mask_warped.clone());
    corners.push_back(img_queue2.corners);
    img_queue2.mtx_corner.unlock();
    feature_match.mtx_cameras.lock();
    feature_match.copy_cameras(cams, feature_match.cameras);
    warped_img_scale = feature_match.warped_image_scale;
    feature_match.mtx_cameras.unlock();
    est_expose.get_cams(cams, warped_img_scale);
    est_expose.get_images(dst1, dst2, workscale);
    est_expose.exposure_compensator_update(); // �����ع���
#else
    //img_queue1.rwc_pop(dst1);
    //img_queue2.rwc_pop(dst2);
    // update ------------------------
    img_queue1.rwc_org_pop(ro1);
    img_queue2.rwc_org_pop(ro2);
    // end ---------------------------
    feature1.mtx_feature.lock();
    workscale = feature1.work_scale;
    feature1.mtx_feature.unlock();
    img_queue1.mtx_corner.lock();
    warped_masks.push_back(img_queue1.mask_warped.clone());
    corners.push_back(img_queue1.corners);
    images_warped.push_back(img_queue1.img_warped.clone());
    img_queue1.mtx_corner.unlock();
    img_queue2.mtx_corner.lock();
    warped_masks.push_back(img_queue2.mask_warped.clone());
    corners.push_back(img_queue2.corners);
    images_warped.push_back(img_queue2.img_warped.clone());
    img_queue2.mtx_corner.unlock();
    feature_match.mtx_cameras.lock();
    feature_match.copy_cameras(cams, feature_match.cameras);
    warped_img_scale = feature_match.warped_image_scale;
    feature_match.mtx_cameras.unlock();
    est_expose.get_cams(cams, warped_img_scale);
    est_expose.get_images(ro1.rwc, ro2.rwc, workscale);
    est_expose.get_rwc_images(vector<Mat>{ro1.rwc, ro2.rwc}, warped_masks, corners, images_warped);
    est_expose.exposure_compensator_update_withrwc(); // �����ع���
#endif

    LOGLN("�ȴ�ƴ�ӷ���������������");
    unique_lock<mutex>lk_seam(est_seam.mtx_esS);
    while (!est_seam.bootflag_seam) {
        est_seam.cond.wait(lk_seam);
    }
    lk_seam.unlock();

    LOGLN("���������ɣ���ʼ����");
    est_seam.mtx_esS.lock();
    est_expose.update_seamed_warpedmask(est_seam.find_mask);
    est_seam.mtx_esS.unlock();

    while (1) {
        if (exit_flag) { break; }

        t = getTickCount();
        // update --------------------------------
        //img_queue1.org_pop(dst1);
        //img_queue2.org_pop(dst2);
        img_queue1.rwc_org_pop(ro1);
        img_queue2.rwc_org_pop(ro2);
        // end -----------------------------------
        feature1.mtx_feature.lock();
        workscale = feature1.work_scale;
        feature1.mtx_feature.unlock();
        img_queue1.mtx_corner.lock();
        warped_masks.push_back(img_queue1.mask_warped.clone());
        corners.push_back(img_queue1.corners);
        img_queue1.mtx_corner.unlock();
        img_queue2.mtx_corner.lock();
        warped_masks.push_back(img_queue2.mask_warped.clone());
        corners.push_back(img_queue2.corners);
        img_queue2.mtx_corner.unlock();
        est_expose.get_images(ro1.org, ro2.org, workscale);
        est_expose.get_feed();
        est_expose.warp_compensate_img();
        if (est_seam.maskflag_seam) {
            est_seam.mtx_esS.lock();
            est_expose.update_seamed_warpedmask(est_seam.find_mask);
            est_seam.mtx_esS.unlock();
            // update------------------------------------
            //img_queue1.rwc_pop(dst1);
            //img_queue2.rwc_pop(dst2);
            // end --------------------------------------
            feature1.mtx_feature.lock();
            workscale = feature1.work_scale;
            feature1.mtx_feature.unlock();
            img_queue1.mtx_corner.lock();
            warped_masks.push_back(img_queue1.mask_warped.clone());
            corners.push_back(img_queue1.corners);
            images_warped.push_back(img_queue1.img_warped.clone());
            img_queue1.mtx_corner.unlock();
            img_queue2.mtx_corner.lock();
            warped_masks.push_back(img_queue2.mask_warped.clone());
            corners.push_back(img_queue2.corners);
            images_warped.push_back(img_queue2.img_warped.clone());
            img_queue2.mtx_corner.unlock();
            feature_match.mtx_cameras.lock();
            feature_match.copy_cameras(cams, feature_match.cameras);
            warped_img_scale = feature_match.warped_image_scale;
            feature_match.mtx_cameras.unlock();
            est_expose.get_cams(cams, warped_img_scale);
            est_expose.get_images(ro1.rwc, ro2.rwc, workscale);
            est_expose.get_rwc_images(vector<Mat>{ro1.rwc, ro2.rwc}, warped_masks, corners, images_warped);
            est_expose.exposure_compensator_update_withrwc(); // �����ع���
        }
        //std::cout << "����عⲹ��" << endl;
#if TIME_LOG   
        LOGLN("�ع⴦��ʱ�� : " << (getTickCount() - t) / getTickFrequency() << " s");
#endif 
    }
    LOGLN("----thread est_exp---- �˳�");

}
/////////////////////////////////////// thread8 ////////////////////////////////////////////////////
void Est_seam() {
    vector<Point>corners;
    vector<UMat> masks_warped_seam{ UMat(),UMat() };
    vector<UMat> images_warped_seam{ UMat(),UMat() };
    Mat dst1, dst2;
    double t;
    LOGLN("�ȴ��������");
    unique_lock<mutex> lk_cameras(feature_match.mtx_cameras);
    while (!feature_match.boot_flag) {
        feature_match.cond.wait(lk_cameras);
    }
    lk_cameras.unlock();
    //////////////////////////////////////////////////////////////////
    unique_lock<mutex> lk_exposure(est_expose.mtx_esE);
    while (!est_expose.bootflag_exposure) {
        est_expose.cond.wait(lk_exposure);
    }
    lk_exposure.unlock();
    //////////////////////////////////////////////////////////////////
    LOGLN("��ʼ����ƴ�ӷ�");
    est_expose.mtx_esE.lock();
    for (uint8_t i = 0;i < CAMERA_NUM;++i) {
        masks_warped_seam[i] = est_expose.masks_warped_f[i].clone();
        images_warped_seam[i] = est_expose.images_warped_f[i].clone();
    }
    corners = est_expose.mycorner;
    est_expose.mtx_esE.unlock();
    est_seam.update_img(images_warped_seam, corners, masks_warped_seam);
    est_seam.init_seamfinder();
    while (1) {
        if (exit_flag) { break; }
        est_expose.mtx_esE.lock();
        for (uint8_t i = 0; i < CAMERA_NUM; ++i) {
            masks_warped_seam[i] = est_expose.masks_warped_f[i].clone();
            images_warped_seam[i] = est_expose.images_warped_f[i].clone();
        }
        corners = est_expose.mycorner;
        est_expose.mtx_esE.unlock();
        est_seam.update_img(images_warped_seam, corners, masks_warped_seam);
        est_seam.init_seamfinder();
        //std::cout << "ƴ�ӷ����" << endl;

    }
    LOGLN("----thread est_seam---- �˳�");
}
void generate_video() {
    VideoWriter writer;
    int frame_fps = 30;
    int frame_width = 720;
    int frame_height = 480;

    writer = VideoWriter("sample.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
        frame_fps, Size(frame_width, frame_height), true);
    std::cout << "frame_width is " << frame_width << endl;
    std::cout << "frame_height is " << frame_height << endl;
    std::cout << "frame_fps is " << frame_fps << endl;
    Mat img;
    for (int i = 1; i < 2000; i++) // ��Ҫ�ϳɵ�ͼ����
    {
        string image_name = "../out_lab/" + to_string(i) + ".jpg";

        img = imread(image_name);
        
        if (img.empty()) {
            cerr << "Failed to read image: " << image_name << endl;
            continue;
        }
        
        if (img.cols != frame_width || img.rows != frame_height) {
            resize(img, img, Size(frame_width, frame_height));
        }
        //cout << i << endl;
        
        if (!img.empty())
        {
            writer << img;
        }
    }
}
/////////////////////////////////////// main ////////////////////////////////////////////////////////
int main() {
#if IsThread
    LOGLN("System supports " << thread::hardware_concurrency() << " concurrent threads.");
    thread video1_input_thread(video_input1, "../left1.mp4");//left video1 load_1
    thread video2_input_thread(video_input2, "../right1.mp4");//right video2 load_2
    thread feature1_thread(find_feature_1);
    thread feature2_thread(find_feature_2);
    thread match_thread(match_feature);
    thread est_exposure_thread(Est_exp);
    thread est_seam_thread(Est_seam);
    thread stitch_thread(stitch_frame);

    video1_input_thread.join();
    video2_input_thread.join();
    feature1_thread.join();
    feature2_thread.join();
    match_thread.join();
    est_exposure_thread.join();
    est_seam_thread.join();
    stitch_thread.join();

    LOGLN("�����������");
#elif videoHave
    generate_video();
#else
    VideoCapture cap1("../left1.mp4");
    if (!cap1.isOpened()) {
        LOGLN("�޷�����Ƶ�ļ�!");
        return -1;
    }
    VideoCapture cap2("../right1.mp4");
    if (!cap2.isOpened()) {
        LOGLN("�޷�����Ƶ�ļ�!");
        return -1;
    }
    Mat frame1, frame2;
    Mat mframe1, mframe2;
    vector<detail::ImageFeatures> features{ detail::ImageFeatures(),detail::ImageFeatures() };
    vector<Mat> imgs{ Mat(),Mat() };
    vector<detail::CameraParams>cams = {
        detail::CameraParams(),
        detail::CameraParams()
    };
    est_expose.init_all();
    int cnt = 0;
    while (1)
    {
        cnt += 1;
        cap1.read(frame1);
        cap2.read(frame2);

        /*string img1_path = "1_img1.jpg";
        string img2_path = "1_img2.jpg";

        frame1 = cv::imread(img1_path,cv::IMREAD_COLOR);
        frame2 = cv::imread(img2_path, cv::IMREAD_COLOR);*/


        img_queue1.org_push(frame1);
        img_queue2.org_push(frame2);

        feature1.find(frame1);
        feature2.find(frame2);

        imgs[0] = feature1.get_dst();
        imgs[1] = feature2.get_dst();
        features[0] = feature1.get_feature();
        features[1] = feature2.get_feature();

        feature_match.update_features_imgs(features, imgs);
        feature_match.match();
        feature_match.est_params();

        img_queue1.warp(frame1, feature_match.cameras[0], feature_match.warped_image_scale);
        img_queue2.warp(frame2, feature_match.cameras[1], feature_match.warped_image_scale);

        // init_exposure
        img_queue1.rwc_pop(mframe1);
        img_queue2.rwc_pop(mframe2);
        feature_match.copy_cameras(cams, feature_match.cameras);
        est_expose.get_cams(cams, feature_match.warped_image_scale);
        est_expose.get_rwc_images(vector<Mat>{mframe1, mframe2}, vector<Mat>{img_queue1.mask_warped.clone(), img_queue2.mask_warped.clone()},
            vector<Point>{img_queue1.corners, img_queue2.corners}, vector<Mat>{img_queue1.img_warped.clone(), img_queue2.img_warped.clone()});
        est_expose.exposure_compensator_update_withrwc(); // �����ع���
        //seam
        //double t = getTickCount();
        est_seam.update_img(vector<UMat>{est_expose.images_warped_f[0].clone(), est_expose.images_warped_f[1].clone()},
            est_expose.mycorner, vector<UMat>{est_expose.masks_warped_f[0].clone(), est_expose.masks_warped_f[1].clone()});
        est_seam.init_seamfinder();
        //std::cout << (getTickCount() - t) / getTickFrequency() * 1000 << endl;
        //expose
        est_expose.update_seamed_warpedmask(est_seam.find_mask);
        est_expose.get_images(frame1, frame2, feature1.work_scale);
        est_expose.warp_compensate_img();
        //blend
        blend_item.update_source(vector<Mat>{est_expose.img_warped_s[0].clone(), est_expose.img_warped_s[1].clone()},
            vector<Mat>{est_expose.mask_warped_blend[0].clone(), est_expose.mask_warped_blend[1].clone()},
            est_expose.corners, est_expose.sizes);
        blend_item.graph_blend();
        cout << cnt << " | ƴ�����" << endl;
        if (img_queue1.org_is_empty() && img_queue2.org_is_empty()) {
            cout << "��Ƶƴ�����" << endl;
            break;
        }
    }

#endif

    return 0;
}