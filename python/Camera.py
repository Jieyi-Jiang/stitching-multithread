import numpy as np
import cv2
import threading

class Camera:
    def __init__(self, name, path, width=1280, height=720, fps=25, fourcc='MJPG'):
        self.name = name
        self.path = path
        self.width = width
        self.height = height
        self.fps = fps
        self.fourcc = cv2.VideoWriter_fourcc(*fourcc)
        self.cap = cv2.VideoCapture(path)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
        self.cap.set(cv2.CAP_PROP_FPS, fps)
        self.cap.set(cv2.CAP_PROP_FOURCC, self.fourcc)
        
        if not self.cap.isOpened():
            raise RuntimeError(f"无法打开摄像头 {name}")
        
        self.frame_count = 0
        self.frame = None
        self.running = False
        self.lock = threading.Lock()  # 添加锁用于线程同步
        self.frame_ready = threading.Event()  # 用于通知新帧可用
        
    def start(self):
        """启动摄像头线程"""
        self.running = True
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()
        
    def stop(self):
        """停止摄像头线程"""
        self.running = False
        self.frame_ready.set()  # 确保等待帧的线程被唤醒
        
        if hasattr(self, 'thread') and self.thread.is_alive():
            self.thread.join(timeout=2.0)
            if self.thread.is_alive():
                print(f"警告: 摄像头 {self.name} 线程未正常退出")
                # 强制终止线程作为最后手段
                # 注意：这可能导致资源未释放，但避免程序崩溃
                self.thread._stop()  # 不推荐但有时必要
        
        # 释放资源
        if self.cap.isOpened():
            self.cap.release()
        print(f"摄像头 {self.name} 已停止")
    
    def _run(self):
        """内部运行方法，不应直接调用"""
        while self.running and self.cap.isOpened():
            try:
                ret, frame = self.cap.read()
                if not ret:
                    print(f"摄像头 {self.name} 读取失败")
                    break
                
                with self.lock:  # 使用锁保护帧的写入
                    self.frame = frame.copy()  # 创建副本以避免数据竞争
                    self.frame_count += 1
                    # print(f"{self.name} : {self.frame_count}")
    
                self.frame_ready.set()  # 通知新帧可用
                
                # 在主线程中显示帧，而不是在摄像头线程中
                # 这样可以避免OpenCV的GUI问题
                
            except Exception as e:
                print(f"摄像头 {self.name} 出错: {e}")
                break
    
    def get_frame(self):
        """获取最新帧"""
        self.frame_ready.wait()  # 等待直到有帧可用
        with self.lock:  # 使用锁保护帧的读取
            if self.frame is not None:
                return self.frame.copy()  # 返回副本以避免数据竞争
            return None
    
    def get_frame_count(self):
        """获取帧计数"""
        with self.lock:  # 使用锁保护计数器的读取
            return self.frame_count
