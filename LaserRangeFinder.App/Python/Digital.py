
import sys
import time
from threading import Thread
from decimal import *
import threading
import serial
import datetime

class Digital(object):
    def __init__(self):
        self.ser = serial.Serial('COM3')
        if self.ser.is_open==False:
            self.ser.open()
        self.ser.baudrate = 9600 # 921600 115200
        self.ser.bytesize = 8
        self.ser.stopbits = 1
        self.ser.parity =serial.PARITY_NONE

    def receiver(self):
        packetLength = 8
   
        buffer = bytearray(packetLength)   
   
        while self.ser.is_open:
            bytesRead = 0;
            while bytesRead < packetLength:
                bytesRead += self.ser.readinto(buffer)
            
            hex_str = ''.join([' 0x{:02X}'.format(x) for x in buffer])
            print(">> %s " % (hex_str))

            pass
        
    def sender(self):
        time.sleep(1)    
        while self.ser.is_open:
            for str1 in ["----", "8.8.8.8.", "tttt", "5toP",str(datetime.datetime.now().second)]:
                buffer =  self.parse_string(str1);
                hex_str = ''.join([' 0x{:02X}'.format(x) for x in buffer])
                print("<< %s " % (hex_str))
                self.ser.write(buffer)
                time.sleep(0.5)
            
            # self.ser.flush()
            time.sleep(1)
       
    def parse_num(self,str1:str):
        addr = 0x01;
        func = 0x06;
        regs = 0x0000;
            
        regs = regs.to_bytes(length=2, byteorder='little', signed=False)
        regs =  [byte for byte in regs]

        num =  int(str1.replace(".", ""))
        num = num.to_bytes(length=2, byteorder='big', signed=False)
        num =  [byte for byte in num]

        result =[addr,func]
        result.extend(regs)
        result.extend(num)
            
        CRC = self.compute_crc(bytes(result));
        CRC = CRC.to_bytes(length=2, byteorder='little', signed=False)
        CRC =  [byte for byte in CRC]

        result.extend(CRC);

        return result;

    def parse_point(self,str1:str):
        addr = 0x01;
        func = 0x06;
        regs = 0x0004;
            
        regs = regs.to_bytes(length=2, byteorder='little', signed=False)
        regs =  [byte for byte in regs]

        p = str1.find('.')

        data = 0 if p == -1  else len(str1) - p - 1;
        data = data.to_bytes(length=2, byteorder='big', signed=False)
        data =  [byte for byte in data]

        result =[addr,func]
        result.extend(regs)
        result.extend(data)
            
        CRC = self.compute_crc(bytes(result));
        CRC = CRC.to_bytes(length=2, byteorder='little', signed=False)
        CRC =  [byte for byte in CRC]

        result.extend(CRC);

        return result;
    
    def parse_decimal(self,str1:str):
        addr = 0x01;
        func = 0x10;
        reg = 0x0090;
        regn = 0x0002;

        reg = reg.to_bytes(length=2, byteorder='big', signed=False)
        reg =  [byte for byte in reg]

        regn = regn.to_bytes(length=2, byteorder='big', signed=False)
        regn =  [byte for byte in regn]

        datan =  0x04
        sign = 0x00 if float(str1)>=0  else 0x01;
        
        p = str1.find('.')
        point = 0 if p == -1  else len(str1) - p - 1;

        str1 =  int(str1.replace(".", ""))
        str1 = str1.to_bytes(length=2, byteorder='big', signed=False)
        str1 =  [byte for byte in str1]

        result =[addr,func]
        result.extend(reg)
        result.extend(regn)
        result.append(datan);
        result.append(sign);
        result.append(point);

        result.extend(str1)
            
        CRC = self.compute_crc(bytes(result));
        CRC = CRC.to_bytes(length=2, byteorder='little', signed=False)
        CRC =  [byte for byte in CRC]

        result.extend(CRC);
        
        return result;

    def parse_string(self,str1:str):
        addr = 0x01;
        func = 0x10;
        reg = 0x0070;
        regn = 0x0003;
        datan = 0x06;
       
        reg = reg.to_bytes(length=2, byteorder='big', signed=False)
        reg =  [byte for byte in reg]

        str1 = str1.encode('ascii');
        datan = len(str1)        
        regn = int(len(str1)/2)
        
        regn = regn.to_bytes(length=2, byteorder='big', signed=False)
        regn =  [byte for byte in regn]

        # str1 = str1.to_bytes(length=datan, byteorder='big', signed=False)
        str1 =  [byte for byte in str1]

        result =[addr,func]
        result.extend(reg)
        result.extend(regn)
        result.append(datan);

        result.extend(str1)
            
        CRC = self.compute_crc(bytes(result));
        CRC = CRC.to_bytes(length=2, byteorder='little', signed=False)
        CRC =  [byte for byte in CRC]

        result.extend(CRC);

        hex_str = ''.join([' 0x{:02X}'.format(x) for x in result])
        print(">> %s " % (hex_str))

        return result;


    def compute_crc(self,data):
        crc = 0xFFFF

        for byte in data:
            crc ^= byte
        
            for _ in range(8):
                if crc & 0x0001:
                    crc >>= 1
                    crc ^= 0xA001
                else:
                    crc >>= 1
        return crc
  
if __name__ == "__main__":
    try:
        led =  Digital()
        # led.parse_num('8888')

        threading.Thread(target=led.receiver).start()
        threading.Thread(target=led.sender).start()
        sys.stdin.readline()
    except Exception as ex:
        print("%s" % ex) 
    finally:
        print("Done.")
       