import numpy as np
import cv2
import Camera 
import time
import signal


# 按下 y，同时保存三张图片到 ../data/group7 目录下，照片名字为 img1.png, img2.png, img3.png
if __name__ == '__main__':
    camera1 = Camera.Camera('camera1', '/dev/video0', 1920, 1080, 25, 'MJPG')
    camera1.start()
    camera2 = Camera.Camera('camera2', '/dev/video2', 1920, 1080, 25, 'MJPG')
    camera2.start()
    camera3 = Camera.Camera('camera3', '/dev/video4', 1920, 1080, 25, 'MJPG')
    camera3.start()
    camera_list = [camera1, camera2, camera3]
    # 创建窗口
    for camera in camera_list:
        cv2.namedWindow(camera.name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(camera.name, 480, 320)
        cv2.waitKey(100)  # 确保窗口创建成功
    while True:
        key = cv2.waitKey(1) & 0xFF
        for i, camera in enumerate(camera_list):
            img = camera.get_frame()
            cv2.imshow(camera.name, img)
        if key == ord('y'):
            for i, camera in enumerate(camera_list):
                img = camera.get_frame()
                cv2.imwrite('../data/group7/img{}.png'.format(i+1), img)
            break
        elif key == ord('q'):
            break
    for camera in camera_list:
        camera.stop()
    cv2.destroyAllWindows()