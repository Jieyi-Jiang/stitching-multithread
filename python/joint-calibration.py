import numpy as np
import cv2
import Camera 
import time
import signal

# 全局退出标志，用于优雅退出
exit_flag = False

def handle_signal(signum, camera_list):
    """处理信号，设置全局退出标志"""
    global exit_flag
    print(f"\n接收到信号 {signum}，正在准备退出...")
    exit_flag = True
    # for camera in camera_list:
    #     camera.exit()
    # cv2.destroyAllWindows()

# 注册信号处理器
signal.signal(signal.SIGINT, handle_signal)  # Ctrl+C
signal.signal(signal.SIGTERM, handle_signal) # 终止信号

def get_pose(obj_pts, img_pts, K, dist):
    # 求每一组图像的外参（这里只用第一张示例，可扩展多张求平均/BA优化）
    rvecs_all, tvecs_all = [], []
    for obj, img in zip(obj_pts, img_pts):
        ok, rvec, tvec = cv2.solvePnP(obj, img, K, dist) 
        if ok:
            print("solvePnP success")
            print("rvec: ", rvec)
            print("tvec: ", tvec)
            rvecs_all.append(rvec)
            tvecs_all.append(tvec)
    return rvecs_all, tvecs_all

def joint_calibration(num=15, corner_size=(12,7), square_size=21):
    """联合标定多个摄像头"""
    # 打开摄像头
    camera1 = Camera.Camera('camera1', '/dev/video0', 1920, 1080, 25, 'MJPG')
    camera1.start()
    camera2 = Camera.Camera('camera2', '/dev/video2', 1920, 1080, 25, 'MJPG')
    camera2.start()
    camera3 = Camera.Camera('camera3', '/dev/video4', 1920, 1080, 25, 'MJPG')
    camera3.start()
    camera_list = [camera1, camera2, camera3]
    mtx_1 = np.array([[743.97677735,   0.        , 935.75657458],
                      [  0.        , 741.26408976, 551.60094992],
                      [  0.        ,   0.        ,   1.        ]])
    dist_1 = np.array([[-0.00628729, -0.03344213, -0.00081306,  0.00060377,  0.00957155]])
    
    mtx_2 = np.array([[7.38542784e+02, 0.00000000e+00, 1.00261851e+03],
                      [0.00000000e+00, 7.35634401e+02, 6.05736231e+02],
                      [0.00000000e+00, 0.00000000e+00, 1.00000000e+00]])
    dist_2 = np.array([[-7.30458153e-03, -3.54923200e-02, -1.88271739e-04,  2.98744583e-05, 1.09485693e-02]]) 
    
    mtx_3 = np.array([[726.86948562,    0.       ,  940.17754387],
                      [  0.         , 724.56580229, 544.12298866],
                      [  0.         ,   0.        ,   1.        ]])
    dist_3 = np.array([[-0.01658264, -0.0261744,  -0.00037458,  0.00018215,  0.00740635]])
    intrinsics = [(mtx_1, dist_1), (mtx_2, dist_2), (mtx_3, dist_3)]

    # 创建窗口
    for camera in camera_list:
        cv2.namedWindow(camera.name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(camera.name, 480, 320)
        cv2.waitKey(100)  # 确保窗口创建成功

    # 检查是否所有摄像头都成功打开
    # print("所有摄像头成功打开")
    # 循环读取摄像头数据，直到接收到退出信号
    
    # 创建角点在世界坐标系中的位置
    # object point temp
    object_point = np.zeros((corner_size[0]*corner_size[1], 3), np.float32)
    object_point[:, :2] = np.mgrid[0:corner_size[0], 0:corner_size[1]].T.reshape(-1, 2) * square_size
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    
    # object points 
    # 棋盘格角点，代表角点在棋盘格坐标系下的位置
    object_points = []
    # 相机角点代表角点在相机坐标系下的位置
    image_points_1 = []
    image_points_2 = []
    image_points_3 = []
    R1_list = []
    R2_list = []
    R3_list = []
    t1_list = []
    t2_list = []
    t3_list = []
    
    cnt = 0
    # 获取角点
    while not exit_flag and cnt < num:
        # 读取所有摄像头数据
        frame_1 = camera1.get_frame()
        frame_2 = camera2.get_frame()
        frame_3 = camera3.get_frame()
        # 检查是否所有摄像头都成功读取数据
        if frame_1 is None or frame_2 is None or frame_3 is None:
            print("读取摄像头数据失败")
            continue
        # 显示摄像头数据
        cv2.imshow(camera1.name, frame_1)
        cv2.imshow(camera2.name, frame_2)
        cv2.imshow(camera3.name, frame_3)
        # 检查是否按下退出键
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            print("退出标定")
            for camera in camera_list:
                camera.stop()   
            cv2.destroyAllWindows()
            return 
        elif key != ord('y'):
            continue

        # 转换为灰度图像
        gray_1 = cv2.cvtColor(frame_1, cv2.COLOR_BGR2GRAY)
        gray_2 = cv2.cvtColor(frame_2, cv2.COLOR_BGR2GRAY)
        gray_3 = cv2.cvtColor(frame_3, cv2.COLOR_BGR2GRAY)
        img_display_1 = frame_1.copy()
        img_display_2 = frame_2.copy()
        img_display_3 = frame_3.copy()
        cv2.putText(img_display_1, "finding the corners ...", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 255, 0), 10)
        cv2.imshow(camera1.name, img_display_1)
        cv2.putText(img_display_2, "finding the corners ...", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 255, 0), 10)
        cv2.imshow(camera2.name, img_display_2)
        cv2.putText(img_display_3, "finding the corners ...", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 255, 0), 10)
        cv2.imshow(camera3.name, img_display_3)
        cv2.waitKey(500)

        # 尝试检测棋盘格角点
        ret_1, corners_1 = cv2.findChessboardCorners(gray_1, corner_size,
                                                flags=cv2.CALIB_CB_ADAPTIVE_THRESH +
                                                        cv2.CALIB_CB_NORMALIZE_IMAGE + 
                                                        cv2.CALIB_CB_FILTER_QUADS)
        ret_2, corners_2 = cv2.findChessboardCorners(gray_2, corner_size,
                                                flags=cv2.CALIB_CB_ADAPTIVE_THRESH +
                                                        cv2.CALIB_CB_NORMALIZE_IMAGE +
                                                        cv2.CALIB_CB_FILTER_QUADS)
        ret_3, corners_3 = cv2.findChessboardCorners(gray_3, corner_size,
                                                flags=cv2.CALIB_CB_ADAPTIVE_THRESH +
                                                        cv2.CALIB_CB_NORMALIZE_IMAGE +
                                                        cv2.CALIB_CB_FILTER_QUADS)
        cv2.imshow(camera1.name, frame_1)
        cv2.imshow(camera2.name, frame_2)
        cv2.imshow(camera3.name, frame_3)
        
        # 检查是否检测到角点
        if ret_1 == False:
                img_display_1 = frame_1.copy()
                print("未检测到棋盘格角点")
                cv2.putText(img_display_1, f"Connot find the corners of {camera1.name}", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 3, (0, 0, 255), 5)
                cv2.imshow(camera1.name, img_display_1)
        if ret_2 == False:
                img_display_2 = frame_2.copy()
                print("未检测到棋盘格角点")
                cv2.putText(img_display_2, f"Connot find the corners of {camera2.name}", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 3, (0, 0, 255), 5)
                cv2.imshow(camera2.name, img_display_2)
        if ret_3 == False:
                img_display_3 = frame_3.copy()
                print("未检测到棋盘格角点")
                cv2.putText(img_display_3, f"Connot find the corners of {camera3.name}", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 3, (0, 0, 255), 5)
                cv2.imshow(camera3.name, img_display_3)
        cv2.waitKey(2000)
        if ret_1 == False or ret_2 == False or ret_3 == False:
            continue
            
        # 如果检测到角点，则进行亚像素精确化
        corners_refined_1 = cv2.cornerSubPix(gray_1, corners_1, (11, 11), (-1, -1), criteria)
        corners_refined_2 = cv2.cornerSubPix(gray_2, corners_2, (11, 11), (-1, -1), criteria)
        corners_refined_3 = cv2.cornerSubPix(gray_3, corners_3, (11, 11), (-1, -1), criteria)
        
        # 显示角点
        img_display_1 = frame_1.copy()
        img_display_2 = frame_2.copy()
        img_display_3 = frame_3.copy()
        img_display_1 = cv2.drawChessboardCorners(img_display_1, corner_size, corners_refined_1, ret_1)
        img_display_2 = cv2.drawChessboardCorners(img_display_2, corner_size, corners_refined_2, ret_2)
        img_display_3 = cv2.drawChessboardCorners(img_display_3, corner_size, corners_refined_3, ret_3)
        status_1 = f"Image {cnt+1}/{num}: VALID (Detected: {len(corners_refined_1)} points)"
        status_2 = f"Image {cnt+1}/{num}: VALID (Detected: {len(corners_refined_2)} points)"
        status_3 = f"Image {cnt+1}/{num}: VALID (Detected: {len(corners_refined_3)} points)"
        cv2.putText(img_display_1, status_1, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(img_display_2, status_2, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(img_display_3, status_3, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.imshow(camera1.name, img_display_1)
        cv2.imshow(camera2.name, img_display_2)
        cv2.imshow(camera3.name, img_display_3)
            
        key = cv2.waitKey(-1) & 0xFF
        if key == ord('y'):
            image_points_1.append(corners_refined_1)
            image_points_2.append(corners_refined_2)
            image_points_3.append(corners_refined_3)
            object_points.append(object_point)
            cnt += 1
            print(f"获取 {cnt}/{num} 个有效帧")
        else:
            cv2.putText(img_display_1, "Give up the frame", (300, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 0, 255), 10)
            cv2.putText(img_display_2, "Give up the frame", (300, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 0, 255), 10)
            cv2.putText(img_display_3, "Give up the frame", (300, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 0, 255), 10)
            cv2.imshow(camera1.name, img_display_1)
            cv2.imshow(camera2.name, img_display_2)
            cv2.imshow(camera3.name, img_display_3)
            cv2.waitKey(2000)
            print(f"放弃当前帧")

    # SolvePnP
    K1 = mtx_1
    dist1 = dist_1
    K2 = mtx_2
    dist2 = dist_2
    K3 = mtx_3
    dist3 = dist_3
    rvecs1, tvecs1 = get_pose(object_points, image_points_1, K1, dist1)
    rvecs2, tvecs2 = get_pose(object_points, image_points_2, K2, dist2)
    rvecs3, tvecs3 = get_pose(object_points, image_points_3, K3, dist3)
    R1_list = rvecs1
    T1_list = tvecs1
    R2_list = rvecs2
    T2_list = tvecs2
    R3_list = rvecs3
    T3_list = tvecs3
    # === 选用第一帧做示例（可以做平均或BA优化） ===
    R1, _ = cv2.Rodrigues(R1_list[0])
    R2, _ = cv2.Rodrigues(R2_list[0])
    R3, _ = cv2.Rodrigues(R3_list[0])
    t1, t2, t3 = T1_list[0], T2_list[0], T3_list[0]

    # === 转换到 cam2 坐标系下 ===    
    # R12 = R2.T @ R1
    # t12 = R2.T @ (t1 - t2)
    
    # R22 = R2.T @ R2
    # t22 = R2.T @ (t2 - t2)

    # R32 = R2.T @ R3
    # t32 = R2.T @ (t3 - t2)
    R12 = R2 @ R1.T
    t12 = t2 - R2 @ R1.T @ t1

    R22 = R2 @ R2.T
    t22 = t2 - R2 @ R2.T @ t2

    R32 = R2 @ R3.T
    t32 = t2 - R2 @ R3.T @ t3

    print("Cam1 -> Cam2 外参:")
    print("R12:\n", R12)
    print("t12:\n", t12)

    print("Cam2 -> Cam2 外参:")
    print("R22:\n", R22)
    print("t22:\n", t22)

    print("Cam3 -> Cam2 外参:")
    print("R32:\n", R32)
    print("t32:\n", t32)
    
    for camera in camera_list:
        camera.stop()
    cv2.destroyAllWindows()
        
        

if __name__ == '__main__':
    joint_calibration(5)
    print("joint_calibration 结束")