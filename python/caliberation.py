'''
(1) read the video frame
(2) show the frame in the window realtime
(3) press 'y' to send the frame to get the object point 
    (3.1) if the corners is found corectly -> go to (4)
    (3.2) if connot find the corners that needed -> go back to (1) and show status 
(4) process corners to sub-pixel pricision 
(5) show the corners in window 
(6) append the vaild conrners and write it to file
    (6.1) if valid frame < required number -> go back to (1)
    (6.2) if valid frame > requried number -> go to (7)
(7) calculate the pramaters
    (7.1) to calculate the internal pramaters of camera, requires more than 3 valid frame
    (7.2) calculate 
(8) show the caliberation result 
(9) calculte the reprojection error
(10) write the result to file
(11) show the frame clibrated in another window
''' 

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

def display_frames(camera_list):
    """在主线程中显示帧"""
    for camera in camera_list:
        cv2.namedWindow(camera.name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(camera.name, 800, 600)
    
    running = True
    while running:
        for camera in camera_list:
            frame = camera.get_frame()
            if frame is not None:
                cv2.imshow(camera.name, frame)
        
        # 统一的键盘检测
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q') or exit_flag:  # 按'q'退出所有摄像头
            running = False
            break
    
    # 停止所有摄像头并销毁窗口
    for camera in camera_list:
        camera.stop()
    cv2.destroyAllWindows()


def caliberation(camera, num=15, corner_size=(12,7), sque_size=21):
    
    # windows 
    window_name = "Caliberation" + camera.name
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(camera.name, 800, 600)
    
    # object point temp
    object_point = np.zeros((corner_size[0]*corner_size[1], 3), np.float32)
    object_point[:, :2] = np.mgrid[0:corner_size[0], 0:corner_size[1]].T.reshape(-1, 2) * sque_size
    
    # object points 
    object_points = []
    image_points = []
    
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    
    cnt = 0
    while cnt < num:
        frame = camera.get_frame()
        if frame is None:
            print("获取帧失败")
            continue
        cv2.imshow(window_name, frame)
        key = cv2.waitKey(1) & 0xFF
        if key == ord('y'):
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            # 尝试检测棋盘格角点
            ret, corners = cv2.findChessboardCorners(gray, corner_size, 
                                                    flags=cv2.CALIB_CB_ADAPTIVE_THRESH + 
                                                        cv2.CALIB_CB_NORMALIZE_IMAGE + 
                                                        cv2.CALIB_CB_FILTER_QUADS)
            if ret == False:
                img_display = frame.copy()
                print("未检测到棋盘格角点")
                cv2.putText(img_display, "Connot find the corners", (100, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 0, 255), 10)
                cv2.imshow(window_name, img_display)
                cv2.waitKey(2000)
                continue
            # 如果检测到角点，则进行亚像素精确化
            corners_refined = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
            
            ############################# 显示交互 ##################################
            img_display = frame.copy()
            status =f"Image {cnt+1}/{num}: VALID (Detected: {len(corners_refined)} points)"
            cv2.drawChessboardCorners(img_display, corner_size, corners_refined, ret)
            cv2.putText(img_display, status, (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.imshow(window_name, img_display)
            key = cv2.waitKey(-1) & 0xFF
            if key == ord('y'):
                object_points.append(object_point)
                image_points.append(corners_refined)
                cnt += 1
                print(f"获取 {cnt}/{num} 个有效帧")
            else:
                cv2.putText(img_display, "Give up the frame", (300, 500), cv2.FONT_HERSHEY_SIMPLEX, 4, (0, 0, 255), 10)
                cv2.imshow(window_name, img_display)
                cv2.waitKey(2000)
                print(f"放弃当前帧")
        
        elif key == ord('q') or exit_flag:
            camera.stop()
            cv2.destroyAllWindows()
            break
        
    camera.stop()
    cv2.destroyAllWindows()
    
    if cnt >= num:
        ret, mtx, dist, rvecs, tvecs = \
            cv2.calibrateCamera( object_points, image_points, gray.shape[::-1], None, None)
        print("\n相机标定结果:")
        print(f"相机矩阵:\n {mtx}")
        print(f"畸变系数: {dist.ravel()}")
        print(f"标定错误: {ret}")
        mean_error = 0
        for i in range(len(object_points)):
            imgpoints2, _ = cv2.projectPoints(object_points[i], rvecs[i], tvecs[i], mtx, dist)
            error = cv2.norm(image_points[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
            mean_error += error
        print(f"\n平均重投影误差: {mean_error / len(object_points):.4f} 像素")
    else:
        print("获取帧数不足，无法进行标定")
        
            
        
def caliberated_frame(camera, mtx, dist):
    window_name_1 = "Caliberated: " + camera.name
    window_name_2 = "Original: " + camera.name
    cv2.namedWindow(window_name_1, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name_1, 800, 600)
    cv2.namedWindow(window_name_2, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name_2, 800, 600)
    while camera.running:
        frame = camera.get_frame()
        h, w = frame.shape[:2]
        frame_cp = frame.copy()
        newcameramtx, roi = cv2.getOptimalNewCameraMatrix(mtx, dist, (w, h), 1, (w, h))
        undistorted = cv2.undistort(frame_cp, mtx, dist, None, newcameramtx)
        x, y, w, h = roi
        undistorted_roi = undistorted[y:y+h, x:x+w].copy()
        if frame is not None:
            cv2.imshow(window_name_1, undistorted_roi)
            cv2.imshow(window_name_2, frame)
        
        # 添加键盘检测
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q') or exit_flag:  # 按'q'退出
            camera.stop()
            cv2.destroyAllWindows()
            break

if __name__ == '__main__':
    try:
        camera1 = Camera.Camera('camera1', '/dev/video0', 1920, 1080, 25, 'MJPG')
        camera1.start()
        camera2 = Camera.Camera('camera2', '/dev/video2', 1920, 1080, 25, 'MJPG')
        camera2.start()
        camera3 = Camera.Camera('camera3', '/dev/video4', 1920, 1080, 25, 'MJPG')
        camera3.start()
        cameras = [camera1, camera2, camera3]
        
        # 在主线程中显示帧
        display_frames(cameras)
        print("display_frames 结束")
        # caliberation(camera1, 5, (11, 8), 20)
        print("caliberation 结束")
        
        mtx_1 = np.array([[743.97677735,   0.        , 935.75657458],
                          [  0.        , 741.26408976, 551.60094992],
                          [  0.        ,   0.        ,   1.        ]])
        dist_1 = np.array([[-0.00628729, -0.03344213, -0.00081306,  0.00060377,  0.00957155]])
        # mtx_1 = np.array([[614.75909275,   0.         ,898.06469883],
        #                   [  0.        , 609.51992427 ,564.02661058],
        #                   [  0.        ,   0.         ,  1.        ]])
        # dist_1 = np.array([[-6.40763539e-03, -7.75676144e-03, -1.15272080e-03, -6.12025739e-05, 1.59816101e-03]])
        
        mtx_2 = np.array([[7.38542784e+02, 0.00000000e+00, 1.00261851e+03],
                          [0.00000000e+00, 7.35634401e+02, 6.05736231e+02],
                          [0.00000000e+00, 0.00000000e+00, 1.00000000e+00]])
        dist_2 = np.array([[-7.30458153e-03, -3.54923200e-02, -1.88271739e-04,  2.98744583e-05, 1.09485693e-02]]) 
        
        mtx_3 = np.array(   [[726.86948562,    0.       ,  940.17754387],
                            [  0.         , 724.56580229, 544.12298866],
                            [  0.         ,   0.        ,   1.        ]])
        dist_3 = np.array([[-0.01658264, -0.0261744,  -0.00037458,  0.00018215,  0.00740635]])
    
        # caliberated_frame(camera1, mtx_1, dist_1)
        # caliberated_frame(camera2, mtx_2, dist_2)
        # caliberated_frame(camera3, mtx_3, dist_3)
    


    except Exception as e:
        print(f"程序出错: {e}")
    finally:
        for camera in cameras:
            # 确保资源被释放
            # if camera in locals():
            if camera.running:
                camera.stop()