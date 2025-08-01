import cv2
import threading
import time
import queue
import os
import signal
import sys
import numpy as np

# 设置Qt平台插件，避免wayland问题
os.environ['QT_QPA_PLATFORM'] = 'xcb'

class Camera:
    def __init__(self, name, path, width, height, fps, fourcc):
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
        self.running = False
        self.frame_queue = queue.Queue(maxsize=1)
        self.lock = threading.Lock()
        self.stop_event = threading.Event()  # 用于通知线程停止
        
    def start(self):
        """启动摄像头线程"""
        with self.lock:
            if self.running:
                return
                
            self.running = True
            self.stop_event.clear()  # 清除停止事件
            self.thread = threading.Thread(target=self._run, daemon=True)
            self.thread.start()
        
    def stop(self):
        """停止摄像头线程"""
        with self.lock:
            if not self.running:
                return
                
            self.running = False
            self.stop_event.set()  # 设置停止事件，通知线程退出
            
        if hasattr(self, 'thread'):
            # 等待线程结束，但设置超时
            self.thread.join(timeout=2.0)
            if self.thread.is_alive():
                print(f"警告: 摄像头 {self.name} 线程未正常退出")
        
        # 释放资源
        if self.cap.isOpened():
            self.cap.release()
    
    def _run(self):
        """内部运行方法，不应直接调用"""
        try:
            while self.running and self.cap.isOpened() and not self.stop_event.is_set():
                ret, frame = self.cap.read()
                if not ret:
                    print(f"摄像头 {self.name} 读取失败")
                    break
                
                self.frame_count += 1
                
                # 使用队列传递帧，如果队列已满则丢弃旧帧
                if self.frame_queue.full():
                    try:
                        self.frame_queue.get_nowait()
                    except queue.Empty:
                        pass
                
                self.frame_queue.put(frame.copy())
                
        except Exception as e:
            print(f"摄像头 {self.name} 出错: {e}")
        finally:
            # 确保在退出前释放资源
            with self.lock:
                if self.cap.isOpened():
                    self.cap.release()
                self.running = False
    
    def get_frame(self):
        """获取最新帧"""
        try:
            return self.frame_queue.get_nowait()
        except queue.Empty:
            return None
    
    def get_frame_count(self):
        """获取帧计数"""
        return self.frame_count

class DisplayManager:
    def __init__(self):
        self.cameras = []
        self.running = False
        self.lock = threading.Lock()
        self.stop_event = threading.Event()
        
    def add_camera(self, camera):
        with self.lock:
            self.cameras.append(camera)
    
    def start(self):
        with self.lock:
            if self.running:
                return
                
            self.running = True
            self.stop_event.clear()
            self.thread = threading.Thread(target=self._run, daemon=True)
            self.thread.start()
        
    def stop(self):
        with self.lock:
            if not self.running:
                return
                
            self.running = False
            self.stop_event.set()
            
        if hasattr(self, 'thread'):
            # 等待线程结束，但设置超时
            self.thread.join(timeout=2.0)
            if self.thread.is_alive():
                print("警告: 显示管理器线程未正常退出")
        
        cv2.destroyAllWindows()
    
    def _run(self):
        """显示线程的主循环"""
        try:
            while self.running and not self.stop_event.is_set():
                with self.lock:
                    cameras = self.cameras.copy()
                
                all_stopped = True
                for camera in cameras:
                    if camera.running:
                        all_stopped = False
                        frame = camera.get_frame()
                        if frame is not None:
                            cv2.imshow(camera.name, frame)
                
                if all_stopped:
                    break
                
                # 处理键盘事件，设置短暂超时以避免长时间阻塞
                key = cv2.waitKey(10) & 0xFF
                if key == ord('q') or key == 27:  # 'q'或ESC键
                    with self.lock:
                        for camera in self.cameras:
                            camera.stop()
                    break
                    
        except Exception as e:
            print(f"显示管理器出错: {e}")
        finally:
            cv2.destroyAllWindows()

def signal_handler(sig, frame):
    """处理中断信号"""
    print("\n收到中断信号，正在关闭程序...")
    global display_manager, cameras
    if 'display_manager' in globals():
        display_manager.stop()
    if 'cameras' in globals():
        for camera in cameras:
            camera.stop()
    sys.exit(0)

def main():
    # 注册信号处理函数
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    global display_manager, cameras
    
    try:
        cameras = [
            Camera('camera1', '/dev/video0', 1920, 1080, 25, 'mp4v'),
            # 可以添加更多摄像头
            # Camera('camera2', '/dev/video1', 1920, 1080, 25, 'mp4v'),
        ]
        
        # 创建显示管理器
        display_manager = DisplayManager()
        
        # 添加摄像头到显示管理器
        for camera in cameras:
            display_manager.add_camera(camera)
            camera.start()
        
        # 启动显示线程
        display_manager.start()
        
        # 主线程等待
        try:
            while True:
                time.sleep(1)
                # 检查所有摄像头是否已停止
                all_stopped = all(not camera.running for camera in cameras)
                if all_stopped and not display_manager.running:
                    break
        except KeyboardInterrupt:
            print("\n收到键盘中断信号")
            signal_handler(None, None)
        
    except Exception as e:
        print(f"程序出错: {e}")
    finally:
        # 确保资源被释放
        if 'display_manager' in globals():
            display_manager.stop()
        if 'cameras' in globals():
            for camera in cameras:
                camera.stop()

if __name__ == '__main__':
    main()
