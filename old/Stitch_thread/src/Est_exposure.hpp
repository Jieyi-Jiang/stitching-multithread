#ifndef EST_EXPOSURE_H
#define EST_EXPOSURE_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/timelapsers.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"
#include <vector>
#include <iostream>
#include <cmath>

#include <condition_variable>
#include <atomic>

using namespace cv;
using namespace cv::detail;
using namespace std;

typedef struct blend_union {
	Mat blend_img1,blend_img2;
	Mat blend_mask1,blend_mask2;
};

class Est_exposure {
// ��һ�ֵ��ã���ʼ������
public:
	// ��������
	atomic<bool> exit_flag{ false };
	mutex mtx_esE;
	condition_variable cond;
	bool bootflag_exposure = false; //��һ��boot��ʶ
	Ptr<ExposureCompensator> compensator; //�ع���
	Ptr<detail::RotationWarper> warper;
	Ptr<WarperCreator> warper_creator = makePtr<cv::SphericalWarper>(); //���ֶ��õ�
	
	// ��������
	vector<detail::CameraParams>cams{
		detail::CameraParams(),
		detail::CameraParams()
	};
	float warped_imagescale; //����cam->focal�ⲿ����
	double work_scale; //����feature�̣߳���ͼ��ͬ��

	// ���
	vector<UMat>masks_warped{ UMat(),UMat() }; //����seam_finder���и���
	vector<UMat>masks_warped_f{ UMat(),UMat() }; //����seam_finder
	vector<UMat>images_warped_f{ UMat(),UMat() }; //����seam_finder---ƴ�ӷ�������ͼ
	vector<Point>mycorner{ Point(),Point() }; //Э��warp--->mask_warped --- ƴ�ӷ������ýǵ�
private:
	// ��������
	std::vector<cv::Mat>images{ Mat(),Mat() }; //���ⲿ����image

	// �м����
	double seam_work_aspect = 1;
	double seam_megapix = 0.1;
	double compose_megapix = 0.6;
	double compose_scale = 1;
	double compose_work_aspect = 1; //��detailǰ��ֲ����ֵδ��
	Mat empty_mat; // ��ʽ��compensator��Ŀǰδ�õ�
	vector<Mat>masks_have{ Mat(),Mat() }; //��������Mat����mask
	vector<UMat>masks{ UMat(),UMat() }; //���ɳ�ʼ����---�ع���feed����
	std::vector<cv::UMat>images_warped{ UMat(),UMat() }; //�ع���feedͼ��
// -----------------------------------------------------------------------------------
// �ڶ��ֵ��ã�Ӧ���ع���
public:
	// ���
	bool bootflag_exposure_done = false; //�ڶ���boot��ʶ
	bool corners_done_flag = false; //�ǵ�ɶ�ȡ��ʶ
	bool graph_done_flag = false; //ͼ������ɶ�ȡ��ʶ
	vector<Mat>img_warped_s{ Mat(),Mat() }; //blendʹ�ã��ع��img
	vector<Mat>mask_warped_blend{ Mat(),Mat() }; //blendʹ�ã����ױ��κ�����
	vector<Mat>use_imgs{ Mat(),Mat() }; //�洢blendʹ��ͼ��
	vector<Mat>use_masks{ Mat(),Mat() }; //�洢blendʹ������
	queue<blend_union> blend_queue; //�洢blendʹ��ͼ��
	vector<Size>sizes{ Size(),Size() }; //�洢compose_scale
	vector<Point>corners; //�洢compose�ǵ�
private:
	// �м����
	bool is_compose_scale_set = false; //һ�ε��ñ�Ϊtrue��ȷ��compose_scale
	bool blend_OK = false; //��ֹ�ڸ����ع�ʱ����ƴ��
	BlocksCompensator* bcompensator; //�ع���ָ��
	int expos_comp_type = ExposureCompensator::GAIN_BLOCKS; //�ع�������
	int expos_comp_nr_feeds = 1;
	int expos_comp_nr_filtering = 2;
	int expos_comp_block_size = 32; //��ʼ���ع���������ֱ�Ӵ�detail��ֲ������������
// -------------------------------------------------------------------------------------
// ��������
public:
	void init_all(); //��������ʼ���ع���
	void exposure_compensator_update(); //�����ع���
	void get_cams(vector<detail::CameraParams>input_cams,float input_warped_scale); //���������
	void get_images(const Mat& input_image1, const Mat& input_image2,double workscale); //��ͼ��
	void get_rwc_images(vector<Mat>rwc_imagesf_in, vector<Mat>rwc_masks_in, vector<Point>Corners_in,vector<Mat>rwc_images_in); //��rwcͼ��
	void warp_compensate_img(); //Ӧ���ع��������շ�������������������
	void update_seamed_warpedmask(vector<UMat>warpedmasks_in); //����seam_finder��������
	void exposure_compensator_update_withrwc();
	void get_feed(); //��ȡ�ع�����ʼ������
	// blend_union������غ��� ----------------------------------
	mutex mtx_bu;
	const uint8_t blend_max_size = 1000;
	void blend_union_push(const blend_union& frame) {
		unique_lock<mutex> lock(mtx_bu);
		//����
		//cond.wait(lock, [this] { return rwc_queue.size() < max_size; });
		//������
		if (blend_queue.size() >= blend_max_size) {
			blend_queue.pop();
		}
		blend_queue.push(bu_clone(frame));  // �����ֹ���ݾ���
		cond.notify_one();
	};
	bool blend_union_pop(blend_union& frame) {
		unique_lock<mutex> lock(mtx_bu);
		cond.wait_for(lock, 1s, [this] { return !blend_queue.empty() || exit_flag; });

		if (blend_queue.empty() || exit_flag)
			return false;

		frame = bu_clone(blend_queue.front());  // ���
		blend_queue.pop();
		cond.notify_one();
		return true;
	};
	// end ------------------------------------------------------
private:
	void init_compensator(); //��ʼ���ع���
	void fill_compensator(); //����ع����������������
	blend_union bu_clone(const blend_union& bu) {
		blend_union thisone;
		thisone.blend_img1 = bu.blend_img1.clone();
		thisone.blend_img2 = bu.blend_img2.clone();
		thisone.blend_mask1 = bu.blend_mask1.clone();
		thisone.blend_mask2 = bu.blend_mask2.clone();
		return thisone;
	}
};


#endif // !EST_EXPOSURE_H
