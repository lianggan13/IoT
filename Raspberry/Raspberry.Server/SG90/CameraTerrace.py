#!/usr/bin/python
#coding:utf-8
import RPi.GPIO as GPIO
import time
PinX = 38
PinY = 40

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

class CameraTerrace:
	def __init__(self):
		GPIO.setup(PinX,GPIO.OUT)
		GPIO.setup(PinY,GPIO.OUT)
		self.pwmX = GPIO.PWM(PinX,50) 	# 50Hz
		self.pwmX.start(0)				# Init DutyCycle
		self.pwmY = GPIO.PWM(PinY,50)
		self.pwmY.start(0)
		time.sleep(0.02)
		self.rx = 90.0
		self.ry = 45.0
		self.MoveX()
		self.MoveY()

	def Left(self):
		r = self.rx + 1.0
		if(self.ValidateAngle(r)==False):
			return
		self.rx = r
		self.MoveX()

	def Right(self):
		r = self.rx - 1.0
		if(self.ValidateAngle(r)==False):
			return
		self.rx = r
		self.MoveX()

	def Up(self):
		r = self.ry - 1.0
		if(self.ValidateAngle(r)==False):
			return
		self.ry = r
		self.MoveY()
	
	def Down(self):
		r = self.ry + 1.0
		if(self.ValidateAngle(r)==False):
			return
		self.ry = r
		self.MoveY()

	def Stop(self):
		self.pwmX.ChangeDutyCycle(0)
		self.pwmY.ChangeDutyCycle(0)

	def MoveX(self):
		dc = self.Angle2DutyCycle(self.rx)
		self.pwmX.ChangeDutyCycle(dc)
		time.sleep(0.02)
		print ("X:%f" % self.rx)

	def MoveY(self):
		dc = self.Angle2DutyCycle(self.ry)
		self.pwmY.ChangeDutyCycle(dc)
		time.sleep(0.02)
		print ("Y:%f" % self.ry)

	def Angle2DutyCycle(self,r:float):
		'''
		SG90 Total T is 20ms (50Hz)
		High T			Angle    DutyCycle
		0.5ms-------------0°	   2.5% 
		1.0ms------------45°	   5.0% 
		1.5ms------------90°	   7.5%
		2.0ms-----------135°	  10.0%
		2.5ms-----------180°	  12.5%
		'''
		return 2.5 + r/360*20
	
	def ValidateAngle(self,r:float):
		if r<0 or r>180:
			print ("Angle must be [0,180]")
			return False
		return True

	def SendCmd(self,cmd):
		try:
			if(cmd.upper() == 'UP'):
				self.Up()
			elif(cmd.upper() == 'LEFT'):
				self.Left()
			elif(cmd.upper() == 'DOWN'):
				self.Down()
			elif(cmd.upper() == 'RIGHT'):
				self.Right()
			elif(cmd.upper() == 'P'):
				self.Stop()
		except KeyboardInterrupt:
			self.pwmX.stop()
			self.pwmY.stop()
			GPIO.cleanup()

if __name__ == "__main__":
	try:
		t = CameraTerrace()
		while True:
			ch = input(">>")
			t.SendCmd(ch)
	except KeyboardInterrupt:
		pass
		GPIO.cleanup()
