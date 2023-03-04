#!/usr/bin/python
#coding:utf-8
#pcf8571  A/D mport smbus
import time
import RPi.GPIO as GPIO
A0 = 0x40
A1 = 0x41
A2 = 0x42
A3 = 0x43
#bus = smbus.SMBus(1)
address = 0x48
D1 = 31
D2 = 33
D3 = 32
POUT = 21
PIN  = 22
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)
GPIO.setup(D2,GPIO.IN)
GPIO.setup(D3,GPIO.IN)

'''HC_SR04'''
GPIO.setup(POUT,GPIO.OUT)
GPIO.setup(PIN,GPIO.IN)
time.sleep(0.02)
GPIO.output(POUT,GPIO.LOW)
time.sleep(0.06)
print ("Setting Trigger pin to zero by default")
time.sleep(0.4)
def read_distance():
	try:
		GPIO.output(POUT,GPIO.HIGH)
		time.sleep(0.000011)
		GPIO.output(POUT,GPIO.LOW)  
		while GPIO.input(PIN) == 0:
			start_time = time.time()
		while GPIO.input(PIN) == 1:
			end_time   = time.time()
		t = end_time - start_time
		distance = 17150 * t
		return distance
	except KeyboardInterrupt:
		pass
		GPIO.cleanup()

'''Flame Senor'''
def isFlame():	
#	bus.write_byte(address,A3)
#	value = bus.read_byte(address)
#	value = (value * 3.3) / 255
#	if value < 2:
#		print ("waring: flame!!! Faleme %.2f"% (value))
#	else:
#		print ("it's ok! value:%.2f" %(value))
#		return 0
	if GPIO.input(D3) == 0:
		print "D3 == 0"
		return 1
	else:
		return 0

'''MQ-2'''
def isFog():
#	bus = smbus.SMBus(1)
#	bus.write_byte(address,A2)
#	value = bus.read_byte(address)
#	value = (value *3300)/255

	if GPIO.input(D2) == 0:
		print "D2 == 0"
		return 1
	else:
		return 0

'''DHT11'''
def read_DHT11():

	data = []
	j = 0

	GPIO.setmode(GPIO.BOARD)

	GPIO.setup(D1, GPIO.OUT)

	GPIO.output(D1, GPIO.LOW)
	time.sleep(0.02)
	GPIO.output(D1, GPIO.HIGH)

	GPIO.setup(D1, GPIO.IN)

	while GPIO.input(D1) == GPIO.LOW:
		continue

	while GPIO.input(D1) == GPIO.HIGH:
		continue

	while j < 40:
		k = 0
		while GPIO.input(D1) == GPIO.LOW:
			continue
		
		while GPIO.input(D1) == GPIO.HIGH:
			k += 1
			if k > 100:
				break
		
		if k < 8:
			data.append(0)
		else:
			data.append(1)

		j += 1


	humidity_bit = data[0:8]
	humidity_point_bit = data[8:16]
	temperature_bit = data[16:24]
	temperature_point_bit = data[24:32]
	check_bit = data[32:40]

	humidity = 0
	humidity_point = 0
	temperature = 0
	temperature_point = 0
	check = 0

	for i in range(8):
		humidity += humidity_bit[i] * 2 ** (7 - i)
		humidity_point += humidity_point_bit[i] * 2 ** (7 - i)
		temperature += temperature_bit[i] * 2 ** (7 - i)
		temperature_point += temperature_point_bit[i] * 2 ** (7 - i)
		check += check_bit[i] * 2 ** (7 - i)

	tmp = humidity + humidity_point + temperature + temperature_point

	if check == tmp:
		#print "temperature : ", temperature, ", humidity : " , humidity
		return temperature,humidity
	else:
		return 999,999
