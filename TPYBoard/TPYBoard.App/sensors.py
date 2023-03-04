import time
import stm

import pyb
from pyb import *
from pyb import UART, Pin, I2C, delay, udelay

def Ao2Voltage(value,maxv=3.3,resolution=10):
    return value*(maxv*1000)/(2^resolution)

class WaterLevel:
    """
    Water level sensor.
    VCC 3.3v
    GND
    AO <--> ADC Port Analog data
    """
    def __init__(self,pin):
        self._pin = pin
        self._adc = pyb.ADC(pin)  # create an analog object from a pin

    def read_data(self):   
        d =   self._adc.read() # read an analog value
        return Ao2Voltage(d);     


class DHT11:
    """
    DHT11 sensor.
    S <--> Pyb Pin
    VCC 3.3~5v
    GDN
    """
    def __init__(self,pin_name):
        pyb.delay(1000)
        self.N1 = Pin(pin_name, Pin.OUT_PP)
        self.PinName=pin_name
        pyb.delay(10)
    def read_data(self):
        # self.__init__(self.PinName)
        data=[]
        j=0
        
        self.N1 = Pin(self.PinName, Pin.OUT_PP)
        # pyb.delay(10)
        
        N1=self.N1
        N1.low()
        delay(20)
        N1.high()
        N1 = Pin(self.PinName, Pin.IN)
        udelay(30)
        if N1.value() != 0:
            return [0,0,"Error"]
        # while N1.value()==1:
        #     continue
        while N1.value()==0:
            continue
        while N1.value()==1:
            continue
        while j<40:
            k=0
            while N1.value()==0:
                continue
            while N1.value()==1:
                k+=1
                if k>100:break
            if k<3:
                data.append(0)
            else:
                data.append(1)
            j=j+1
        j=0
        humidity_bit=data[0:8]
        humidity_point_bit=data[8:16]
        temperature_bit=data[16:24]
        temperature_point_bit=data[24:32]
        check_bit=data[32:40]
        humidity=0
        humidity_point=0
        temperature=0
        temperature_point=0
        check=0
        for i in range(8):
            humidity+=humidity_bit[i]*2**(7-i)
            humidity_point+=humidity_point_bit[i]*2**(7-i)
            temperature+=temperature_bit[i]*2**(7-i)
            temperature_point+=temperature_point_bit[i]*2**(7-i)
            check+=check_bit[i]*2**(7-i)
        tmp=humidity+humidity_point+temperature+temperature_point
        message=''
        if check==tmp:
            message = 'temperature is',temperature,'-wet is',humidity,'%'
        else:
            message = 'Error:',humidity,humidity_point,temperature,temperature_point,check
        return [str(temperature),str(humidity),str(message)]


class Rainfall:
    """
    Rainfall Ray sensor (FC-37)
    VCC 
    GND
    DO <--> GPIO  Digital data
    AO <--> ADC Port Analog data
    if value is low than defined data, DO value is 0, 
    if value is high than defined data, DO value is 1.
    AO is the specific value.
    """
    def __init__(self,do_pin,ao_pin):
        self._dpin =  Pin(do_pin, Pin.IN, Pin.PULL_UP)
        self._ao_pin = ao_pin
        self._adc = pyb.ADC(ao_pin)

    def read_data(self):
        # 8bit --> 255，10bit --> 1023，12bit --> 4095
        do = self._dpin.value()
        ao =self._adc.read()

        return [do, Ao2Voltage(ao)]

class MCU:
    """
    MCU STM32
    MCU temperature 
    MCU VBAT
    MCU VREF
    """
    def __init__(self,resolution = 12):
        # resolution: 8/10/12
        self._adc = pyb.ADCAll(resolution)    # create an ADCAll object,分辨率（resolution=12）

    def read_data(self):  
        return self._pin.value()

    def read_data(self):
        # channel: 16-temp 17-verf 18-ba
        # channel = 16
        # val = self._adc.read_channel(channel) # read the given channel
        temp = self._adc.read_core_temp()      # read MCU temperature
        vbat = self._adc.read_core_vbat()      # read MCU VBAT  后备电池电压（1.21v参考）
        verf = self._adc.read_core_vref()      # read MCU VREF  3.3V电源作为参考基准电压
        
        return [temp,vbat,verf]

class PIR:
    """
    PassiveInfrared Ray: HC-SR501 
    VCC 
    Signal <--> GPIO(IN)
    GND
    if motion sensor value is 1, otherwise 0.
    """
    def __init__(self,pin):
        self._pin =  Pin(pin, Pin.IN, Pin.PULL_UP)

    def read_data(self):  
        return self._pin.value()


class SoilMoisture:
    """
    Soil moisture sensor.
    VCC 
    GND
    DO <--> GPIO  Digital data
    AO <--> ADC Port Analog data
    if value is low than defined data, DO value is 0, 
    if value is high than defined data, DO value is 1.
    AO is the specific value.
    Sensor value reference:
        0 ~300 : dry soil
        300~700 : wet soil 
        700~950 : in water
    """
    def __init__(self,do_pin,ao_pin):
        self._dpin =  Pin(do_pin, Pin.IN, Pin.PULL_UP)
        self._ao_pin = ao_pin
        self._adc = pyb.ADC(ao_pin)

    def read_data(self):
        # 8bit --> 255，10bit --> 1023，12bit --> 4095
        do = self._dpin.value()
        ao =self._adc.read()

        return [do, Ao2Voltage(ao)]


class LightIntensity:
    """
    Light intensity sensor(GY-30) <--> I2C(1)
    SDA <--> X10
    SCL <--> X9
    VCC
    GND
    ADO(ADDR/address) <--> GND
    """
    def __init__(self,id=1):
        self.accel_addr = 35
        self.i2c = pyb.I2C(id)
        self.i2c.init(pyb.I2C.MASTER, baudrate=400000)
        print(self.i2c.scan())

    def start(self):
        self.i2c.scan()
        self.i2c.is_ready(self.accel_addr)
        self.i2c.send(1,35)
        self.i2c.send(0x10,35)

    def read_data(self):
        self.i2c.scan()
        self.i2c.is_ready(self.accel_addr)
        self.data=self.i2c.mem_read(2, self.accel_addr, 0x47, timeout=1000,addr_size=8)
        data =  self.data[0]*0xff+self.data[1]
        return data

    # # Define some constants from the datasheet

    # DEVICE     = 0x23 # The value is 0x23 if GY-30's ADO(ADDR) pin is connected to GND, value is 0x5c while VCC.

    # POWER_DOWN = 0x00 # No active state
    # POWER_ON   = 0x01 # Power on
    # RESET      = 0x07 # Reset data register value

    # # Start measurement at 4lx resolution. Time typically 16ms.
    # CONTINUOUS_LOW_RES_MODE = 0x13
    # # Start measurement at 1lx resolution. Time typically 120ms
    # CONTINUOUS_HIGH_RES_MODE_1 = 0x10
    # # Start measurement at 0.5lx resolution. Time typically 120ms
    # CONTINUOUS_HIGH_RES_MODE_2 = 0x11
    # # Start measurement at 1lx resolution. Time typically 120ms
    # # Device is automatically set to Power Down after measurement.
    # ONE_TIME_HIGH_RES_MODE_1 = 0x20
    # # Start measurement at 0.5lx resolution. Time typically 120ms
    # # Device is automatically set to Power Down after measurement.
    # ONE_TIME_HIGH_RES_MODE_2 = 0x21
    # # Start measurement at 1lx resolution. Time typically 120ms
    # # Device is automatically set to Power Down after measurement.
    # ONE_TIME_LOW_RES_MODE = 0x23


    # i2c = I2C(1, I2C.MASTER)             # create and init as a master

    # #i2c.is_ready(0x23)           # check if slave 0x23 is ready
    # #i2c.scan()                   # scan for slaves on the bus, returning

    # def convertToNumber(data):
    #     # Simple function to convert 2 bytes of data
    #     # into a decimal number
    #     #return ((data[1] + (256 * data[0])) / 1.2)
    #     #convert float to int
    #     return int(((data[1] + (256 * data[0])) / 1.2))

    # def readLight(addr=DEVICE):
    #     #  data = bus.read_i2c_block_data(addr,ONE_TIME_HIGH_RES_MODE_1)
    #     i2c.send(CONTINUOUS_HIGH_RES_MODE_1, DEVICE) 
    #     time.sleep(0.2)  #Waiting for the sensor data
    #     data = i2c.mem_read(8, DEVICE, 2) # read 3 bytes from memory of slave 0x23, tarting at address 2 in the slave
    #     #print(data)
    #     #print(data[1])
    #     #print(data[2])
    #     return convertToNumber(data)






