import time
import _thread
from pyb import UART, Pin #,RTC

from TOFSense_F import *
from digital import *

try:
    tof = TOFSense_F(no=1)
    dig = Digital(no=3)
    time.sleep(1)
    last = 0.0
    while True:
        curent = tof.query()
        # if curent == None or curent == 0:  
        if curent == None: #or curent == 0:  
            # dig.display('----') 
            time.sleep(0.5)
            continue

        disstr = str("{:.3f}".format(curent))
        if abs(curent - last) > 0.030: # ±3cm
            last = curent
            dig.display(disstr)
        
        if curent < 0.100: # 10cm
            dig.display('5toP') 
            time.sleep(1)
            dig.display(disstr)

        time.sleep(0.5)

except Exception as ex:
    print("%s" % ex)