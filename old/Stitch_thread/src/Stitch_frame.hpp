#ifndef BLENDIMG_H
#define BLENDIMG_H

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
#include <filesystem>
using namespace std;
using namespace cv;
using namespace cv::detail;

class graph_blender {
public:
	// ��������
	bool try_cuda = false;

	// ����
	vector<Mat>images_warped{ Mat(),Mat() }; //�ں�ͼ��
	vector<Mat>masks_warped{ Mat(),Mat() }; //�ں�����
	vector<Point>corners{ Point(),Point() }; //ͼ��ǵ�
	vector<Size>sizes{ Size(),Size() }; //ͼ���С

	// ���
	Mat result, result_mask; //�ںϺ�ͼ������
	int id = 0;
	// ����
	void update_source(vector<Mat>img_warped_s_in, vector<Mat>mask_warped_in, vector<Point>corner_in, vector<Size>size_in); //����source
	void graph_blend(); //�ںϲ�����
	void save_img();
private:
	// �м����
	int n = 0; //����
	int blend_type = Blender::MULTI_BAND; //�ں�������
	float blend_strength = 5; //detail��ֲ��������ʼ���ں���

	// ����
	void try_blend(); //�����ں�
	void draw_result(); //������
	void save_maskresult(); //������ƴ�ӷ���
};

#endif // !BLENDIMG_H