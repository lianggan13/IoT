#!/usr/bin/python
#coding:utf-8

#注:HC-SR04 vcc必须为 标准5V电压
import RPi.GPIO as GPIO
import time
POUT = 21
PIN  = 22
class HCSR04:
	def __init__(self):
		GPIO.setmode(GPIO.BOARD)
		GPIO.setwarnings(False)
		print ("Meauring Distance")
		GPIO.setup(POUT,GPIO.OUT)
		GPIO.setup(PIN,GPIO.IN)
		time.sleep(0.02)
		GPIO.output(POUT,GPIO.LOW)
		time.sleep(0.06)
		print ("Setting Trigger pin to zero by default")
		time.sleep(0.4)
		
	def read_distance(self):
		try:
			# 产生一个13us的触发信号
			GPIO.output(POUT,GPIO.HIGH)  #GPIO.HIGH
			time.sleep(0.000011)
			GPIO.output(POUT,GPIO.LOW)  #GPIO.LOW
		
			#计算回波信号的持续时间
			while GPIO.input(PIN) == 0:
				start_time = time.time()
			while GPIO.input(PIN) == 1:
				end_time   = time.time()
			t = end_time - start_time

			#计算距离
			distance = 17150 * t
			#print ("Measured Distance is: %.1f " %distance,"cms.")
			return distance
		except KeyboardInterrupt:
			pass
			GPIO.cleanup()
		


		
