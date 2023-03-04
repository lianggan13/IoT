import math
import time

import pyb
from pyb import *

from sensors import *
from LoRa import *

def osc(n, d):
    for i in range(n):
        pyb.hid((0, int(20 * math.sin(i / 10)), 0, 0))
        pyb.delay(d)

def SwitchDemo():
    def echo():
        delay(10)
        if(sw()):
            print("user button clicked")
    sw = Switch()
    sw.callback(echo)
# sw.callback(null)

def AccelDemo():
    xBases = (5,-5)

    xlignts = (LED(2),LED(3))
    ylights = (LED(1),LED(4))


    accel = Accel()
    while True:
        x = accel.x() # -30 ~ 30
        # y = accel.y() #
        # z = accel.z() #
        if x > xBases[0]:
            xlignts[0].on()
            xlignts[1].off()
        elif x < xBases[1]:
            xlignts[0].off()
            xlignts[1].on()
        else:
            xlignts[0].off()
            xlignts[1].off()
        delay(10)

def SensorTest():
    #osc(100, 50)
    #LED(4).toggle()
    wl = WaterLevel(pyb.Pin.board.X1)
    dht = DHT11(pyb.Pin.board.X2)
    rain = Rainfall(pyb.Pin.board.X3,pyb.Pin.board.X4)
    mcu = MCU()
    # TODO: Test
    # pir = PIR()
    # pir.read_data()
    # soil = SoilMoisture(pyb.Pin.board.X3,pyb.Pin.board.X4)
    # soil.read_data()
    # lighty = LightIntensity()
    # data=lighty.read_data()
    while True:
        data = wl.read_data()
        print('Water level: ',data,'mv')

        data = dht.read_data()
        print('DHT11: ',data)

        data = rain.read_data()
        print('Rainfall: ',data[0],data[1],'mv')

        data = mcu.read_data()
        print('MCU: ',data)

        pyb.delay(1000)

if __name__ == "__main__":
    # SensorTest()
    lora = LoRa()
    #lora.start_URAT1()
    lora.start_URAT2()
    lora.Test()
