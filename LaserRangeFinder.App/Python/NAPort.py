import sys
import time
from threading import Thread
import threading
import serial

ser = serial.Serial('COM3')
if ser.is_open==False:
    ser.open()
    
class TOFSense_F(object):
    def __init__(self):
        ser.baudrate = 9600 # 921600 115200
        ser.bytesize = 8
        ser.stopbits = 1
        ser.parity =serial.PARITY_NONE

    def receiver(self):
        packetLength = 16
   
        buffer = bytearray(packetLength)   
   
        while ser.is_open:
            bytesRead = 0;
            while bytesRead < packetLength:
                bytesRead += ser.readinto(buffer)
            # buffer.extend(temp)
      
            if buffer[0] == 0x57:
                # hex_str = hex(int.from_bytes(buffer, 'little'))[2:]
                hex_str = ''.join([' {:02X}'.format(x) for x in buffer])
                print(">> %s " % (hex_str))
            
                timeArr =  buffer[4:8]
                disArr =[buffer[8],buffer[9],buffer[10],0x00]  # dateframe[8:11]
                statusArr = buffer[11:12]
                signalStreagthArr = buffer[12:14]
                rangePrecisionArr = buffer[14:15]
                sumChkArr = buffer[15:16]

                systime = int.from_bytes(bytes(timeArr),'little')  
                dis = int.from_bytes(bytes(disArr), 'little') *1.0/1000.0 
                status = int.from_bytes(bytes(statusArr), 'little') 
                signalStreagth = int.from_bytes(bytes(signalStreagthArr), 'little') 
                rangePrecision = int.from_bytes(bytes(rangePrecisionArr), 'little') 
                # sumChk = int.from_bytes(bytes(sumChkArr), 'little') 
                print("time(ms):            %s" % (systime))
                print("dis(m):              %s" % (dis))
                print("dis_status:          %s" % (status))
                print("signal_strength:     %s" % (signalStreagth))
                print("range_precision(cm): %s" % (rangePrecision))
            
                pass
    def sender(self):
        time.sleep(1)    
        while ser.is_open:
            readframe = [0x57, 0x10,0xFF,0xFF,0x00,0xFF,0xFF,0x63]
            buffer =  bytes(readframe)
            hex_str = ''.join([' {:02X}'.format(x) for x in buffer])
            print("<< %s " % (hex_str))
    
            ser.write(buffer)
            # ser.flush()
        
            time.sleep(0.02)

  
if __name__ == "__main__":
    try:
        tof =  TOFSense_F()
        threading.Thread(target=tof.receiver).start()
        threading.Thread(target=tof.sender).start()
        sys.stdin.readline()
    except Exception as ex:
        print("%s" % ex) 
    finally:
        print("Done.")
       