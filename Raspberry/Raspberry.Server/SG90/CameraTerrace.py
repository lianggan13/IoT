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
		self.pwmX.start(2.5)		    # Duty: 0% or 2.5% --> Angle: 0°
		self.pwmY = GPIO.PWM(PinY,50)
		self.pwmY.start(2.5)
		time.sleep(0.02)
		self.rx = 40.0
		self.ry = 40.0
		self.MoveX()
		
		self.MoveY()

	def Left(self):
		angle = self.rx + 5.0
		if(self.ValidateAngleX(angle)==False):
			return
		self.rx = angle
		self.MoveX()

	def Right(self):
		angle = self.rx - 5.0
		if(self.ValidateAngleX(angle)==False):
			return
		self.rx = angle
		self.MoveX()

	def Up(self):
		angle = self.ry - 5.0
		if(self.ValidateAngleY(angle)==False):
			return
		self.ry = angle
		self.MoveY()
	
	def Down(self):
		angle = self.ry + 5.0
		if(self.ValidateAngleY(angle)==False):
			return
		self.ry = angle
		self.MoveY()

	def MoveX(self):
		self.SetAngle(PinX,self.pwmX,self.rx)
		print (":%f" % self.rx)

	def MoveY(self):
		self.SetAngle(PinY,self.pwmY,self.ry)
		print ("Y:%f" % self.ry)

	def SetAngle(self,servoPin,pwm,angle:float):
		'''
		SG90 Total T is 20ms (50Hz)
		High T			Angle    DutyCycle
		0.5ms-------------0°	   2.5% 
		1.0ms------------45°	   5.0% 
		1.5ms------------90°	   7.5%
		2.0ms-----------135°	  10.0%
		2.5ms-----------180°	  12.5%
		'''
		duty =  angle/360*20 + 2.5
		print ("Duty:%f%%" % duty)
		# GPIO.output(servoPin, True)
		pwm.ChangeDutyCycle(duty)
		time.sleep(0.5)
		# GPIO.output(servoPin, False)
		pwm.ChangeDutyCycle(0)  # 停止PWM
		time.sleep(0.1)
	
	def ValidateAngleX(self,angle:float):
		if angle<-5 or angle>75:
			print ("Angle must be [-5,75]")
			return False
		return True
	
	def ValidateAngleY(self,angle:float):
		if angle<-10 or angle>105:
			print ("Angle must be [-10,105]")
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
		except KeyboardInterrupt:
			self.pwmX.stop()
			self.pwmY.stop()
			GPIO.cleanup()

if __name__ == "__main__":
	try:
		t = CameraTerrace()
		for i in range(0,180):
			input(">> (Enter)")
			t.SendCmd('LEFT')

		# while True:
		# 	ch = input(">>")
		# 	t.SendCmd(ch)
	except KeyboardInterrupt:
		pass
		GPIO.cleanup()
