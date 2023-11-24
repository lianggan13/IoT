import time
import _thread
from pyb import UART, Pin #,RTC

from TOFSense_F import *
from digital import *

try:
    tof = TOFSense_F(no=1)
    dig = Digital(no=3)
    time.sleep(1)
    dis = 0.0
    dig.display('5toP')
    while True:
        dis2 = tof.query()
        # print("dis: %s" % (dis))
        # print("dis2: %s" % (dis2))
        # print("dis2 - dis: %s" % (dis2 - dis))

        if dis2 != None and abs(dis2 - dis) > 0.030: # ±3cm
            dis = dis2
            disstr = str("{:.3f}".format(dis))
            print("=== disstr(m): %s" % (disstr))
            dig.display(disstr)
            # dig.display('5toP') # 10cm

            if dis < 0.100: 
                time.sleep(0.5)
                dig.display('5toP') # 10cm

            time.sleep(1)
            # time.sleep(0.02)
            continue
        time.sleep(1)

except Exception as ex:
    print("%s" % ex)