
import time
import _thread
from pyb import UART, Pin #,RTC

class TOFSense_F:
    """
    UART(1) is on:(TX, RX) = (X9, X10) = (PB6,  PB7)
    UART(2) is on:(TX, RX) = (X3, X4)  = (PA2,  PA3)
    UART(3) is on:(TX, RX) = (Y9, Y10) = (PB10, PB11)
    UART(4) is on:(TX, RX) = (X1, X2)  = (PA0,  PA1)
    UART(6) is on:(TX, RX) = (Y1, Y2)  = (PC6,  PC7)
    """
    def __init__(self,no=1):
        print("URAT(%s)..." % no)   
        self.uart = UART(no,9600)  # UART1: X9 X10
        self.uart.init(9600, bits=8, parity=None, stop=1,timeout=1500,flow=0,timeout_char=0,
                       read_buf_len=64)
    def start_query(self):
        _thread.start_new_thread(self.query,())

    def query(self):
        buffer = [0x57, 0x10,0xFF,0xFF,0x00,0xFF,0xFF,0x63]
        hex_str = ''.join([' 0x{:02X}'.format(x) for x in buffer])
        print("<< %s " % (hex_str))
        d = bytearray(buffer)
        bytesRead = self.uart.write(d)

        # time.sleep(0.02)

        print("Wait recv blocked...")
        bytesRead = self.uart.any()  # Wait read blocked
        if(bytesRead > 0): 
            # buffer = self.uart.read()
            buffer = bytearray(bytesRead)
            bytesRead =  self.uart.readinto(buffer,len(buffer))
            # print(">> %s. len = %s" % (buffer, bytesRead))   
            hex_str = ''.join([' 0x{:02X}'.format(x) for x in buffer])
            print(">> %s " % (hex_str))

            key = 0x57
            i = self.find_index(buffer,key)
            if i != -1:
                # index = buffer.index(key)
                i = self.find_index(buffer,key)
                i = i+8

                disArr =  [buffer[i],buffer[i+1],buffer[i+2],0x00]
                dis = int.from_bytes(bytes(disArr), 'little') *1.0/1000.0 

                return dis
            else:
                print(">> Error")
                return None
            
            timeArr =  buffer[4:8]
            disArr =[buffer[8],buffer[9],buffer[10],0x00]
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
            # print("time(ms):            %s" % (systime))
            # print("dis(m):              %s" % (dis))
            # print("dis_status:          %s" % (status))
            # print("signal_strength:     %s" % (signalStreagth))
            # print("range_precision(cm): %s" % (rangePrecision))
            return dis
        else:
            print(">> (None)")

        return None
    
    def find_index(self, bytearr, element):
        for i in range(len(bytearr)):
            if bytearr[i] == element:
                return i
        return -1


if __name__ == "__main__":
    try:
        tof = TOFSense_F()
        time.sleep(1)
        while True:
            dis =  tof.query()
            print("dis(m): %s" % (dis))
            time.sleep(1)
    except Exception as ex:
        print("%s" % ex) 
    finally:
        _thread.exit()