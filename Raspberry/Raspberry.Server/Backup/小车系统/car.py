#!/usr/bin/python
# -*- coding:UTF-8 -*-
import RPi.GPIO as GPIO
import time
INT1 = 10
INT2 = 11
INT3 = 13
INT4 = 15
B1 = 37
B2 = 39   #GND
GPIO.setwarnings(False)
#M1 左电机 INT1 INT2 TOU1 TOU2
#M2 右电机 INT3 INT4 TOU3 TOU4

class 	CarCtrl:
	def __init__(self):
		GPIO.setmode(GPIO.BOARD)
		GPIO.setup(INT1,GPIO.OUT)
		GPIO.setup(INT2,GPIO.OUT)
		GPIO.setup(INT3,GPIO.OUT)
		GPIO.setup(INT4,GPIO.OUT)
		GPIO.setup(B1,GPIO.OUT)
		self.onLight = False
		self.stop()
	
	def init(self):
		GPIO.setmode(GPIO.BOARD)
		GPIO.setup(INT1,GPIO.OUT)
		GPIO.setup(INT2,GPIO.OUT)
		GPIO.setup(INT3,GPIO.OUT)
		GPIO.setup(INT4,GPIO.OUT)
		

	def right(self):# M1 正转   
		GPIO.output(INT1,GPIO.LOW)
		GPIO.output(INT2,GPIO.HIGH)
		GPIO.output(INT3,GPIO.LOW)
		GPIO.output(INT4,GPIO.LOW)

	def left(self):#M2 正转    
		GPIO.output(INT1,GPIO.LOW)
		GPIO.output(INT2,GPIO.LOW)
		GPIO.output(INT3,GPIO.HIGH)
		GPIO.output(INT4,GPIO.LOW)

	def forward(self):#M1 M2 正转
		GPIO.output(INT1,GPIO.LOW)
		GPIO.output(INT2,GPIO.HIGH)
		GPIO.output(INT3,GPIO.HIGH)
		GPIO.output(INT4,GPIO.LOW)

	def back(self):#M1 M2 反转
		GPIO.output(INT1,GPIO.HIGH)
		GPIO.output(INT2,GPIO.LOW)
		GPIO.output(INT3,GPIO.LOW)
		GPIO.output(INT4,GPIO.HIGH)

	def light(self):
		if self.onLight == False:
			GPIO.output(B1,GPIO.HIGH)
			self.onLight = True
		else:
			GPIO.output(B1,GPIO.LOW)
			GPIO.output(B1,GPIO.LOW)
			self.onLight = False

	def stop(self):
		GPIO.output(INT1,GPIO.LOW)
		GPIO.output(INT2,GPIO.LOW)
		GPIO.output(INT3,GPIO.LOW)
		GPIO.output(INT4,GPIO.LOW)

	def commend(self,cmd):
		try:
			if(cmd == 'w' or cmd == 'W'):
				self.forward()
			elif(cmd == 's' or cmd == 'S'):
				self.back()
			elif(cmd == 'a' or cmd == 'A'):
				self.left()
			elif(cmd == 'd' or cmd == 'D'):
				self.right()
			elif(cmd == 'f' or cmd == 'F'):
				self.light()
			elif(cmd == '0'):
				self.stop()
		except KeyboardInterrupt: 
			pass
			GPIO.cleanup()
if __name__ == "__main__":
	try:
		car = CarCtrl()
		while True:
			data = raw_input(">>")
			car.commend(data)
	except KeyboardInterrupt: 
		pass
		GPIO.cleanup()
