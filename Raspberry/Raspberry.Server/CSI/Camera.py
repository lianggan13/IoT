#!/usr/bin/python3
# -*- coding:UTF-8 -*-

from threading import Thread
import sys,time,_thread
import datetime
import socket
import asyncio
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
                # camera.resolution = (300,200)
                camera.resolution = (640,480)
                # camera.resolution = (1920,1080)
                camera.framerate = 30
                #camera.start_preview()
                time.sleep(2)
                while True:
                    stream = io.BytesIO()
                    for foo in camera.capture_continuous(stream,format='jpeg',use_video_port=True):
                        # Truncate the stream to the current position (in case
                        # prior iterations output a longer image)
                        stream.seek(0)
                        data = stream.read()
                        
                        # end =  bytes("\0", encoding = "utf-8")
                        # if(self.capured_event != None):
                        #     self.capured_event(self,data + end)

                        # data = bytes("1", encoding = "utf-8")
                        # head = (4 + len(data)).to_bytes(4, 'big') 

                        if(self.capured_event != None):
                            self.capured_event(self, data)    
                        '''
                        n = stream.tell()
                        n = len(data)
                        image = Image.open(stream)
                        print('Image is %dx%d' % image.size)
                        image.verify() # Image is verified
                        image = Image.open(image_stream) # open --> verify --> open --> save
                        image.save('a1.jpeg',format('JPEG'))
                        '''
                        stream.seek(0)
                        stream.truncate()
                        
                        # time.sleep(0.1)
          
      
if __name__ == "__main__":
    camera =  Camera()
    camera.start()