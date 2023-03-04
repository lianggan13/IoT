#!/usr/bin/python
#coding:utf-8
import RPi.GPIO as GPIO
import time
POUTX = 40
POUTY = 26
DC_SET = 1.5
DC_SETY = 1
GPIO.setwarnings(False)
class SG90:
	def __init__(self):
		self.dc_x = 7.5
		self.dc_y = 7.5
		GPIO.setmode(GPIO.BOARD)
		GPIO.setup(POUTX,GPIO.OUT)
		GPIO.setup(POUTY,GPIO.OUT)
		self.pwm_x = GPIO.PWM(POUTX,50)
		self.pwm_x.start(0)
		self.pwm_x.ChangeDutyCycle(6)
		time.sleep(0.2)
		self.pwm_x.ChangeDutyCycle(0)

		self.pwm_y = GPIO.PWM(POUTY,50)
		self.pwm_y.start(0)
		self.pwm_y.ChangeDutyCycle(0.5)
		time.sleep(0.2)
		self.pwm_y.ChangeDutyCycle(0)
		
	def up(self):
		if self.dc_y > 1:
			self.dc_y -= DC_SETY	
			self.pwm_y.ChangeDutyCycle(self.dc_y)
			time.sleep(0.2)
			self.pwm_y.ChangeDutyCycle(0)
		else:
			self.pwm_y.ChangeDutyCycle(0)
		print ("up:*************%f************" % self.dc_y)
	
	def down(self):
		if self.dc_y < 12:
			self.dc_y += DC_SETY
			self.pwm_y.ChangeDutyCycle(self.dc_y)
			time.sleep(0.2)
			self.pwm_y.ChangeDutyCycle(0)
		else:
			self.pwm_y.ChangeDutyCycle(0)
		print ("down:*************%f*************" % self.dc_y)

	def left(self):
		if self.dc_x < 11:
			self.dc_x += DC_SET
			self.pwm_x.ChangeDutyCycle(self.dc_x)
			time.sleep(0.2)
			self.pwm_x.ChangeDutyCycle(0)
		else:
			self.pwm_x.ChangeDutyCycle(0)
		print ("left:*************%f*************" % self.dc_x)

	def right(self):
		if self.dc_x > 1.5:
			self.dc_x -= DC_SET	
			self.pwm_x.ChangeDutyCycle(self.dc_x)
			time.sleep(0.2)
			self.pwm_x.ChangeDutyCycle(0)
		else:
			self.pwm_x.ChangeDutyCycle(0)
		print ("right:*************%f************" % self.dc_x)

	def commend(self,cmd):
		try:
			if(cmd == 'j' or cmd == 'J'): # j
				self.left()
			elif(cmd == 'l' or cmd == 'L'): # l
				self.right()
			elif(cmd == 'k' or cmd == 'K'): # k
				self.down()
			elif(cmd == 'i' or cmd == 'I'):  # i
				self.up()
		except KeyboardInterrupt:
			pass 
			pwm_x.stop()
			pwm_y.stop()
			GPIO.cleanup()

if __name__ == "__main__":
	try:
		t = SG90()
		while True:
			ch = raw_input(">>")
			t.commend(ch)
	except KeyboardInterrupt:
		pass
		GPIO.cleanup()
