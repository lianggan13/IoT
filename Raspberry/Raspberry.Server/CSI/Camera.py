#!/usr/bin/python3
# -*- coding:UTF-8 -*-

from threading import Thread
import sys,time,_thread
import datetime
import socket
import asyncio
import struct
import picamera
import io
from PIL import Image
import asyncio
from Utils.Event import *

class Camera:
    """
    CSI Camera
    """
    def __init__(self) -> None:
        self.capured_event = Event()
        self.isplaying = False
    def Start(self):
        ''' google...
        https://blog.csdn.net/talkxin/article/details/50504601
        https://picamera.readthedocs.io/en/release-1.13/api_streams.html
        https://picamera.readthedocs.io/en/release-1.13/recipes2.html#rapid-capture-and-streaming
        '''
        with picamera.PiCamera() as camera:
                camera.vflip = True
                camera.hflip = True
                #camera.resolution = (640,480)  # VGA 分辨率 
                camera.resolution = (320, 240) # QVGA 分辨率
                camera.framerate = 25       # 视频帧率(帧/s)
                camera.image_effect = 'denoise' # 降噪 
                # camera.annotate_background = picamera.Color('black') # 黑色背景 
                #camera.annotate_text_size = 20 # 小号字体
                #camera.iso = 100 # 设置 ISO 感光度:较低的ISO可以降低画质但影响不大
                
                camera.exposure_mode = 'sports'  # 关闭自动曝光 # 'auto' # 自动曝光  'sports' # 运动模式曝光,更快速的自动曝光 
                #camera.awb_mode = 'off' # 禁用自动白平衡来加快摄像头初始化
                #camera.format = 'rgb'   # RGB格式比JPEG质量差但计算更简单

                # xxxx...
                # camera.focus_mode = 'fixed'
                # camera.focus_autothreshold  = 'fixed' # 固定对焦
                #camera.quality  = 10        # 视频编码质量(取值范围为 0-40，值越小，画面质量越低)
                #camera.bitrate  = 1000000   # 视频比特率 bps(比特率越低，画面质量越低)
                #camera.still_stats.bitrate = 1000000 # 设置图像捕获的比特率,单位是比特/秒  
                #camera.video_stats.bitrate = 2000000 # 设置视频录制的比特率
                #camera.image_stabilization = False # 关闭图像稳定可以提高性能,但画面会抖动
                #camera.jpeg_quality = 50 # 降低JPG图像的质量压缩参数,范围是1-100,默认是85
                #camera.use_gpu = True   # 启用GPU来进行图像预处理,可能会提高性能
                
                #camera.start_preview()
                self.isplaying = True
                time.sleep(2)
                try:
                    stream = io.BytesIO()
                    for _ in camera.capture_continuous(stream,format='jpeg',use_video_port=True):
                        # Truncate the stream to the current position (in case
                        # prior iterations output a longer image)
                        if(self.isplaying == False):
                            break
                        # 获取当前帧的数据
                        frame_data = stream.getvalue()

                        # 获取当前帧的长度
                        frame_length = len(frame_data)

                        # 将当前帧直接转为字节数组
                        length_data =  frame_length.to_bytes(4, byteorder='big')
                        # 将当前帧的长度打包为4字节的无符号整数
                        #length_data = struct.pack('!I', frame_length)
                        
                        if(self.capured_event != None):
                            self.capured_event(self, length_data)
                            self.capured_event(self, frame_data)

                        '''
                        n = stream.tell()
                        n = len(data)
                        image = Image.open(stream)
                        print('Image is %dx%d' % image.size)
                        image.verify() # Image is verified
                        image = Image.open(image_stream) # open --> verify --> open --> save
                        image.save('a1.jpeg',format('JPEG'))
                        '''

                        # 清空流以进行下一帧捕获
                        stream.seek(0)
                        stream.truncate()
                except Exception as ex:
                    print("%s" % ex)
                finally:
                    camera.close()

    def Stop(self):
        self.isplaying = False
               

if __name__ == "__main__":
    camera =  Camera()
    camera.Start()