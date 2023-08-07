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

    def start(self):
        ''' google...
        https://blog.csdn.net/talkxin/article/details/50504601
        https://picamera.readthedocs.io/en/release-1.13/api_streams.html
        https://picamera.readthedocs.io/en/release-1.13/recipes2.html#rapid-capture-and-streaming
        '''
        with picamera.PiCamera() as camera:
                camera.vflip = True
                camera.hflip = True
                camera.resolution = (640,480)
                camera.quality  = 10        # 视频编码质量(取值范围为 0-40，值越小，画面质量越低)
                camera.bitrate  = 1000000   # 视频比特率 bps(比特率越低，画面质量越低)
                camera.framerate = 25       # 视频帧率(帧/s)
                #camera.start_preview()
                time.sleep(2)
                try:
                    stream = io.BytesIO()
                    for _ in camera.capture_continuous(stream,format='jpeg',use_video_port=True):
                        # Truncate the stream to the current position (in case
                        # prior iterations output a longer image)
                        
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

if __name__ == "__main__":
    camera =  Camera()
    camera.start()