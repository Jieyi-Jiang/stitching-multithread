import cv2

# 0 通常代表默认的摄像头。如果你有多个摄像头，可以尝试 1, 2, ...
# 在 Linux 上，/dev/video0 对应 0，/dev/video1 对应 1，以此类推。
cap = cv2.VideoCapture("/dev/video4")
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920) 
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
cap.set(cv2.CAP_PROP_FPS, 25)
ret = cap.get(cv2.CAP_PROP_FPS)


cv2.namedWindow('Camera 3', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Camera 3', 640, 480)
# 检查摄像头是否成功打开
if not cap.isOpened():
    print("错误：无法打开摄像头")
    exit()

# 创建一个无限循环来持续读取视频帧
while True:
    # ret 是一个布尔值，如果帧读取成功，则为 True，否则为 False
    # frame 是读取到的图像帧（一个 NumPy 数组）
    ret, frame = cap.read()

    # 如果帧读取失败（例如，摄像头断开连接），则退出循环
    if not ret:
        print("错误：无法接收帧，正在退出...")
        break

    # 在窗口 'Live Camera Feed' 中显示图像帧
    cv2.imshow('Camera 3', frame)

    # 等待 1 毫秒，检查是否有按键按下
    # 如果按下的是 'q' 键，则退出循环
    # ord('q') 获取 'q' 键的 ASCII 码
    if cv2.waitKey(1) == ord('q'):
        break

# 循环结束后，释放摄像头资源
cap.release()

# 关闭所有 OpenCV 创建的窗口
cv2.destroyAllWindows()
