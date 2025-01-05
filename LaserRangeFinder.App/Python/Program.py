import sys
import time
from threading import Thread
import threading
import serial
from Digital import *

readframe = [0xF2,0x57,0x00,0xFF,0x00,0x98,0x98,0x45,0x01,0x55,0x00,0x00,0x01,0x06,0x04,0x02]
buffer =  bytes(readframe)
# index = int(str(readframe.index(0x57)))

key = 0x57
if key in buffer:
	index = buffer.index(key)
	index = index+8

	dis =  buffer[index:index+3]
	dis.append(0x00)

led =  Digital()
result =  led.parse_num('8888')

hex_str = ''.join([' 0x{:02X}'.format(x) for x in result])
print(">> %s " % (hex_str))

print("Done.")
sys.stdin.readline()






