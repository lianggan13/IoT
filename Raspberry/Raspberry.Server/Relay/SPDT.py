#!/usr/bin/python
# -*- coding:UTF-8 -*-
from ast import Str
import RPi.GPIO as GPIO
import time
INT = 11

GPIO.setwarnings(False)

class SPDT:
	def __init__(self):
		GPIO.setmode(GPIO.BOARD)
		GPIO.setup(INT,GPIO.OUT)
		self.OFF()

	def ON(self):
		GPIO.output(INT,GPIO.HIGH) 

	def OFF(self):
		GPIO.output(INT,GPIO.LOW)
	

if __name__ == "__main__":
	try:
		spdt = SPDT()
		while True:
			data = input(">>(1:NO 0:OFF)")
			if(data.upper() == '1'):
				spdt.ON()
			elif(data.upper() == '0'):
				spdt.OFF()
	except KeyboardInterrupt: 
		GPIO.cleanup()
