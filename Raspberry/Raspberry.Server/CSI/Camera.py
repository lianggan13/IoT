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
                camera.quality  = 10        # ��Ƶ��������(ȡֵ��ΧΪ 0-40��ֵԽС����������Խ��)
                camera.bitrate  = 1000000   # ��Ƶ������ bps(������Խ�ͣ���������Խ��)
                camera.framerate = 25       # ��Ƶ֡��(֡/s)
                #camera.start_preview()
                time.sleep(2)
                try:
                    stream = io.BytesIO()
                    for _ in camera.capture_continuous(stream,format='jpeg',use_video_port=True):
                        # Truncate the stream to the current position (in case
                        # prior iterations output a longer image)
                        
                        # ��ȡ��ǰ֡������
                        frame_data = stream.getvalue()

                        # ��ȡ��ǰ֡�ĳ���
                        frame_length = len(frame_data)

                        # ����ǰֱ֡��תΪ�ֽ�����
                        length_data =  frame_length.to_bytes(4, byteorder='big')
                        # ����ǰ֡�ĳ��ȴ��Ϊ4�ֽڵ��޷�������
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

                        # ������Խ�����һ֡����
                        stream.seek(0)
                        stream.truncate()
                except Exception as ex:
                    print("%s" % ex)
                finally:
                    camera.close()

if __name__ == "__main__":
    camera =  Camera()
    camera.start()