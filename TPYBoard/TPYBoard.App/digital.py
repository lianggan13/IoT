
import time
import _thread
from pyb import UART, Pin #,RTC

class Digital:
    """
    UART(1) is on:(TX, RX) = (X9, X10) = (PB6,  PB7)
    UART(2) is on:(TX, RX) = (X3, X4)  = (PA2,  PA3)
    UART(3) is on:(TX, RX) = (Y9, Y10) = (PB10, PB11)
    UART(4) is on:(TX, RX) = (X1, X2)  = (PA0,  PA1)
    UART(6) is on:(TX, RX) = (Y1, Y2)  = (PC6,  PC7)
    """
    def __init__(self,no=3):
        print("URAT(%s)..." % no)   
        self.uart = UART(no,9600)
        self.uart.init(9600, bits=8, parity=None, stop=1,timeout=1500,flow=0,timeout_char=0,
                       read_buf_len=128)

    def display(self,str1:str):
        print("Display: %s" % (str1))
        buffer = self.parse_string(str1)
        # buffer = self.parse_decimal(str1)
        d = bytearray(buffer)
        bytesRead = self.uart.write(d)

        print("Wait recv blocked...")
        bytesRead = self.uart.any()  # Wait read blocked
        if(bytesRead > 0): 
            # buffer = self.uart.read()
            buffer = bytearray(bytesRead)
            self.uart.readinto(buffer)
            # print(">> %s. len = %s" % (buffer, len))   
            hex_str = ''.join([' 0x{:02X}'.format(x) for x in buffer])
            # print(">> %s " % (hex_str))
            return buffer
        else:
            print(">> (None)")
        
        return None

    def parse_decimal(self,str1:str):
        addr = 0x01
        func = 0x10
        reg = 0x0090
        regn = 0x0002

        reg = reg.to_bytes(2, 'big')
        reg =  [byte for byte in reg]

        regn = regn.to_bytes(2, 'big')
        regn =  [byte for byte in regn]

        datan =  0x04
        sign = 0x00 if float(str1)>=0  else 0x01
        
        p = str1.find('.')
        point = 0 if p == -1  else len(str1) - p - 1

        str1 =  int(str1.replace(".", ""))
        str1 = str1.to_bytes(2, 'big')
        str1 =  [byte for byte in str1]

        result =[addr,func]
        result.extend(reg)
        result.extend(regn)
        result.append(datan)
        result.append(sign)
        result.append(point)

        result.extend(str1)
            
        CRC = self.compute_crc(bytes(result))
        CRC = CRC.to_bytes(2, 'little')
        CRC =  [byte for byte in CRC]

        result.extend(CRC)
        
        hex_str = ''.join([' 0x{:02X}'.format(x) for x in result])
        print("<< %s " % (hex_str))

        return result

    def parse_string(self,str1:str):
        addr = 0x01
        func = 0x10
        reg = 0x0070
        regn = 0x0003
        datan = 0x06
        
        reg = reg.to_bytes(2, 'big')
        reg =  [byte for byte in reg]
        
        if len(str1) % 2 != 0:
            str1 = str1 + " "
        str1 = str1.encode('ascii')
        datan = len(str1)
        regn = int(len(str1) / 2)
        # print("datan: %s " % (datan))
        # print("regn: %s " % (regn))
        
        regn = regn.to_bytes(2, 'big')
        regn =  [byte for byte in regn]

        # str1 = str1.to_bytes(datan, 'big')
        str1 =  [byte for byte in str1]

        result =[addr,func]
        result.extend(reg)
        result.extend(regn)
        result.append(datan)

        result.extend(str1)
            
        CRC = self.compute_crc(bytes(result))
        CRC = CRC.to_bytes(2, 'little')
        CRC =  [byte for byte in CRC]

        result.extend(CRC)

        hex_str = ''.join([' 0x{:02X}'.format(x) for x in result])
        # print("<< %s " % (hex_str))

        return result

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
        dig = Digital(no=3)
        time.sleep(1)
        while True:
            for str1 in ["----", "8.8.8.8.", "tttt", "5toP"]:
                dig.display(str1)
                # dig.display("0.1")
                time.sleep(1)
    except Exception as ex:
        print("%s" % ex) 
    finally:
        _thread.exit()