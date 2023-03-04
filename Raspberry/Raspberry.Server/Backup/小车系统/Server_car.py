#!/usr/bin/python
#coding:utf-8
from socket import *
import time
import spidev as SPI
from BMP180 import BMP180
import SSD1306
import Senor,car,sg90
import Image
import ImageDraw
import ImageFont
import math
import smbus
import json
import threading
import struct
import os
import cv2.cv as cv
import Image,StringIO
import picamera
#########################tcpsocket########################
HOST = ""
PORT = 6666
ADDR = (HOST,PORT)
BUFFSIZE = 1024
sockfd = socket(AF_INET,SOCK_STREAM)
sockfd.setsockopt(SOL_SOCKET,SO_REUSEADDR,1)
sockfd.bind(ADDR)
sockfd.listen(13)
#########################devices###################
bmp = BMP180()  #BMP180 i2c
servo = sg90.SG90()  #SG90 
car = car.CarCtrl()     #L289N
#########################spioled######################
# Raspberry Pi pin configuration:
RST = 35
# Note the following are only used with SPI:
DC = 36
bus = 0
device = 0

# 128x64 display with hardware SPI:
disp = SSD1306.SSD1306(RST, DC, SPI.SpiDev(bus,device))

# Initialize library.
disp.begin()

# Clear display.
disp.clear()
disp.display()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
#128x64
width = disp.width #128 
height = disp.height #64
#创建image 画布 对象
image = Image.new('1', (width, height))
image2 = Image.open('swpu.png').resize((width, height),Image.ANTIALIAS).convert('1')
# Get drawing object to draw on image.
# 创建draw 画布 对象
draw = ImageDraw.Draw(image)

# Draw a black filled box to clear the image.
draw.rectangle((0,0,width,height), outline=0, fill=0)

# Draw some shapes.
# First define some constants to allow easy resizing of shapes.
padding = 3
top = padding
x = padding+1
# Load default font.
font = ImageFont.load_default()

# Alternatively load a TTF font.
# Some other nice fonts to try: http://www.dafont.com/bitmap.php
#font = ImageFont.truetype('Minecraftia.ttf', 8)

# Write two lines of text.
#draw.text((x, top),    'This is first line', font=font, fill=255)
disp.image(image2)
disp.display()
'''
while True:
	draw.rectangle((0,0,width,height),outline=0,fill=0)
	draw.text((x, top+3),str(mytime), font=font, fill=255)
	draw.text((x, top+20),'temperature: '+str(temp), font=font, fill=255)
	draw.text((x,top+30),'pressure:'+str(pressure),font=font,fill=255)
	draw.text((x,top+40),'altitude:'+str(altitude),font=font,fill=255)

	# Display image.
	disp.image(image)
	disp.display()
	time.sleep(1);
'''
class SendPicCSI(threading.Thread):
	def __init__(self):
		super(SendPicCSI,self).__init__()
	def run(self):
		try:
			print "pinUSB waiting..............."
			evt3.wait()
			connfd,addr = sockfd.accept()
			print "SendPicCSI connected from %s:%s..." % addr
			camera = picamera.PiCamera()
			camera.resolution = (300, 200)
			camera.vflip = True
			camera.hflip = True
			while True:
				#os.system("raspistill -o imgCSI.jpg -t 50 -w 300 -h 200 -rot 180")
				#with picamera.PiCamera() as camera:
				camera.capture("imgCSI.jpg")
				fhead=os.path.getsize('imgCSI.jpg')
				fhead = struct.pack('l',fhead)
				connfd.send(fhead)
				with open('imgCSI.jpg','rb') as fp:
					for data in fp:
						connfd.send(data)
				time.sleep(0.05)
		except:
			connfd.close()
			print 'SendPicCSI from %s:%s closed!!!' % addr
		
class SendPic(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		try:
			print "pinUSB waiting..............."
			capture = cv.CaptureFromCAM(0)  
			cv.SetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_WIDTH, 640)  
			cv.SetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_HEIGHT, 480)  
			evt2.wait()
			connfd,addr = sockfd.accept()  
			print "SendPicUSB connected from %s:%s..." % addr
			evt3.set()
			while True: 
				img = cv.QueryFrame(capture)  
				pi = Image.fromstring("RGB", cv.GetSize(img), img.tostring())  
				buf = StringIO.StringIO() 
				pi.save(buf, format = "JPEG")  
				jpeg = buf.getvalue()  
				buf.close()  
				transfer = jpeg.replace("\n", "\-n")
				connfd.sendall(transfer + "\n")  
				time.sleep(0.05)  
		except:
			print 'SendPicUSB from %s:%s closed!!!' % addr
  			connfd.close()
			
class SendThread(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		try:
			connfd,addr = sockfd.accept()
			print 'SendSenor connected from %s:%s' % addr
			evt.set()		#释放信号量
			while True:
				#temp = bmp.read_temperature()  temp板子上的上的温度
				temp_2,humi = Senor.read_DHT11()#temp_2 DHT11 传感器的温度
				pressure =math.trunc(bmp.read_pressure()/100.0)
				altitude =math.trunc(bmp.read_altitude())
				distance = math.trunc(Senor.read_distance())
				flame 	 =Senor.isFlame()
				fog  	 = Senor.isFog()
				
				if distance > 500:
					continue
				data = struct.pack('iiiiiii',temp_2,humi,pressure,altitude,distance,flame,fog)
				connfd.send(data)
				time.sleep(0.5)
					
			connfd.close()
		except:
			connfd.close()
			print 'SendSenor from %s:%s closed!!!' % addr

def main():
	try:
			print "main waiting.,,,"
			evt.wait()  #申请信号量
			connfd,addr = sockfd.accept()
			print 'main connected from %s:%s' % addr
			evt2.set()
			while True:
				data = connfd.recv(BUFFSIZE)
				if not data:
					continue
				servo.commend(data)  # 舵机控制
				car.commend(data)		# 行驶控制
			connfd.close()
	except:
		connfd.close()
		print 'main from %s:%s closed!!!' % addr
		
evt	 = threading.Event()  			#控制主线程的开启
evt2 = threading.Event() 			#控制USB视频线程的开启
evt3 = threading.Event()			#控制CSI视频线程的开启

if __name__ == "__main__":
	try:
		sendthread 	= SendThread()
		sendpic  	= SendPic()
		sendpicCSI 	= SendPicCSI()
		sendthread.start()
		sendpic.start()
		sendpicCSI.start()
		main()
		#os.system("./mjpg.sh")  #shell脚本:mjpg-steamer程序
	except EOFError,KeyboardInterrupt:
		sendthread.stop()
		recvthread.stop()
		GPIO.cleanup()


