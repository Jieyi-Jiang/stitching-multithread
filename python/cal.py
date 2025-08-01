import numpy as np
import cv2
import glob
import os

# 创建输出目录保存带角点的图像
output_dir = "detected_corners"
os.makedirs(output_dir, exist_ok=True)

# 设置棋盘格参数（修改为你的棋盘格尺寸）
chessboard_size = (10, 7)       # 内角点数量 (列, 行)
square_size = 23.0              # 每个方格的实际尺寸（毫米）


# 准备3D对象点，对应于棋盘格上每个内角点的位置
objp = np.zeros((chessboard_size[0] * chessboard_size[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:chessboard_size[0], 0:chessboard_size[1]].T.reshape(-1, 2) * square_size
print(objp)


# 存储标定数据
objpoints = []  # 3D点
imgpoints = []  # 2D点
valid_images = []  # 成功检测角点的图像路径

# 读取所有标定图像
image_paths = glob.glob('calibration_images_2/*.jpg') + glob.glob('calibration_images/*.png')

if not image_paths:
    print("未找到图像！请检查路径：'calibration_images/'")
    exit()

print(f"找到 {len(image_paths)} 张图像，开始检测角点...")

# 1. 角点检测阶段
for i, fname in enumerate(image_paths):
    img = cv2.imread(fname)
    if img is None:
        print(f"无法读取图像: {fname}")
        continue
        
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    # 尝试检测棋盘格角点
    ret, corners = cv2.findChessboardCorners(gray, chessboard_size, 
                                            flags=cv2.CALIB_CB_ADAPTIVE_THRESH + 
                                                  cv2.CALIB_CB_NORMALIZE_IMAGE + 
                                                  cv2.CALIB_CB_FILTER_QUADS)
    
    # 处理检测结果
    if ret:
        # 亚像素级优化
        criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
        corners_refined = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
        
        # 绘制并显示角点
        img_display = img.copy()
        cv2.drawChessboardCorners(img_display, chessboard_size, corners_refined, ret)
        
        # 添加状态信息
        status = f"Image {i+1}/{len(image_paths)}: VALID (Detected: {len(corners_refined)} points)"
        print(status)
        cv2.putText(img_display, status, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        
        # 保存有效数据
        objpoints.append(objp)
        imgpoints.append(corners_refined)
        valid_images.append(fname)
    else:
        img_display = img.copy()
        status = f"Image {i+1}/{len(image_paths)}: INVALID (No corners found)"
        cv2.putText(img_display, status, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    
    # 显示图像
    # cv2.imshow(window_title, img_display)
    cv2.imwrite(f"./detected_corners_2/Image{i+1}-Corner.jpg", img_display)

# 2. 标定计算阶段（至少需要3张有效图像）
if len(valid_images) < 3:
    print(f"\n错误：只有 {len(valid_images)} 张图像检测到角点，至少需要3张!")
    exit()

print(f"\n成功检测 {len(valid_images)} 张图像的角点，开始相机标定...")

# 执行相机标定
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(
    objpoints, imgpoints, gray.shape[::-1], None, None)

# 3. 结果展示
print("\n相机标定结果:")
print(f"相机矩阵:\n {mtx}")
print(f"畸变系数: {dist.ravel()}")
print(f"标定错误: {ret}")

# 计算重投影误差
mean_error = 0
for i in range(len(objpoints)):
    imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
    error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
    mean_error += error
print(f"\n平均重投影误差: {mean_error / len(objpoints):.4f} 像素")

# 4. 可视化校正效果（使用最后一张有效图像）
sample_img = cv2.imread("./test_2.jpg")
h, w = sample_img.shape[:2]

# 计算优化后的相机矩阵
newcameramtx, roi = cv2.getOptimalNewCameraMatrix(mtx, dist, (w, h), 1, (w, h))

# 校正畸变
undistorted = cv2.undistort(sample_img, mtx, dist, None, newcameramtx)

cv2.putText(undistorted, "Undistorted Image", (50, 50), 
           cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)
cv2.imwrite("./cal_2.jpg", undistorted)
cv2.imshow("Distortion Correction Result", undistorted)
cv2.waitKey(0)

# 5. 保存标定结果
np.savez("camera_calibration.npz", 
        mtx=mtx, 
        dist=dist, 
        newcameramtx=newcameramtx,
        roi=roi,
        reprojection_error=mean_error/len(objpoints))

print("\n标定结果已保存到 camera_calibration.npz")
cv2.destroyAllWindows()

