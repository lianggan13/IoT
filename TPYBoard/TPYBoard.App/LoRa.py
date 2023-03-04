#    LoRa.py
#Communication module: LoRa.
#Communication method with gateway via LoRa.
#Uart port drive LoRa module.
#Parse JSON between device and gateway via LoRa channel.
#LoRa module: E32-TTL-100
#Pin specification:
#Module         MCU
#M0(IN)    <--> GPIO(OUT)     #mode setting, can not hang
#M1(IN)    <--> GPIO(OUT)     #mode setting, can not hang
#RXD(IN)   <--> X1(TX)(OUT)   #UART4
#TXD(OUT)  <--> X2(RX)(IN)    #UART4
#AUX(OUT)  <--> GPIO/INT(IN)  #module status detecting
#VCC
#GND

#Communication mode is 0, need to set M0 and M1 to 0.

import time

#JSON data format:
#{ID:123,CMD:heartbeat,DATA:hello,SEQUENCE:123}
from pyb import UART, Pin #,RTC


class LoRa:
    """
    """
    def __init__(self):
        pass
        # self.rtc = RTC()

    def start_URAT1(self):
        print("Start URAT1...")
        M0 = Pin(Pin.board.Y3, Pin.OUT_PP)
        M1 = Pin(Pin.board.Y4, Pin.OUT_PP)
        M0.low()
        M1.low()
        self.u1 = UART(1,9600)  
        self.u1.init(9600, bits=8, parity=None, stop=1,timeout=3000)

    def start_URAT2(self):
        print("Start URAT2...")
        M0 = Pin(Pin.board.X5, Pin.OUT_PP)
        M1 = Pin(Pin.board.X6, Pin.OUT_PP)
        M0.low()
        M1.low()
        self.u2 = UART(2,9600)  
        self.u2.init(9600, bits=8, parity=None, stop=1,timeout=3000)

    def start_URAT6(self):
        print("Start URAT6...")
        M0 = Pin(Pin.board.Y3, Pin.OUT_PP)
        M1 = Pin(Pin.board.Y4, Pin.OUT_PP)
        M0.low()
        M1.low()
        self.u6 = UART(6,9600)  
        self.u6.init(9600, bits=8, parity=None, stop=1,timeout=3)  
        
        # time.sleep(1)
        # self.rtc.wakeup(1000,lambda t: self.send())
    
        

    # def send(self):
    #         # (year, month, day, weekday, hours, minutes, seconds, subseconds)
    #         # dt = self.rtc.datetime()
    #         # sdt = '{yyyy}-{mm:02d}-{dd:02d} {hh:02d}:{MM:02d}:{ss:02d}'.format(
    #         #         yyyy = dt[0],mm = dt[1],dd = dt[2],hh = dt[4], MM = dt[5],ss = dt[6]
    #         # )

    #         # print('send: {ID:1,CMD:OnLine,DATA:%s}' % sdt)
    #         self.u6.write('{ID:1,CMD:OnLine,DATA:ssss}')

    def Test(self):
        print("start test...")
        i = 0
        while True:
            d = '{ID:1,CMD:OnLine,DATA: %d}' % i
            print("send data:  %s" % d)
            n = self.u6.write(d)
            print("write done. len = %s" % n)
            time.sleep(0.3)
            continue
            #time.sleep(0.3)
            len = self.u2.any()
            if(len > 0): 
                print("read data: %s. len = %s" % (self.u2.read(), len))   
            else:
                print("read no Data...")
            print("--------------")
            i += 1
            time.sleep(1)

            # continue
            # len = self.u1.any() # wait blocked
            # if(len > 0): 
            #     print("data coming...")
            #     print(self.u1.readline())
            #     self.u6.write('{ID:1,CMD:OnLine,DATA:ssss}')
            #     time.sleep(1)
            # else:
            #     print("No Data...")
            #     self.u6.write('{ID:1,CMD:OnLine,DATA:ssss}')
            # time.sleep(2)

if __name__ == "__main__":
    # SensorTest()
    lora = LoRa()
    # lora.start_URAT1()
    lora.start_URAT2()
    lora.start_URAT6()
    time.sleep(1)
    lora.Test()