import time
import utime
import _thread
from pyb import UART, Pin  # ,RTC

from TOFSense_F import *
from digital import *
# from NRF24L01.nrf24l01use import *
from NRF24L01 import *
from NRF24L01.NRF24L01 import *
from NRF24L01.NRF24L01_2 import *


# from NRF24L01 import NRF24L01
# from NRF24L01.NRF24L01 import *


def Test_TOFSense_F():
    try:
        tof = TOFSense_F(no=1)
        dig = Digital(no=3)
        time.sleep(1)
        last = 0.0
        while True:
            curent = tof.query()
            # if curent == None or curent == 0:
            if curent == None:  # or curent == 0:
                # dig.display('----')
                time.sleep(0.5)
                continue

            disstr = str("{:.3f}".format(curent))
            if abs(curent - last) > 0.030:  # ±3cm
                last = curent
                dig.display(disstr)

            if curent < 0.100:  # 10cm
                dig.display('5toP')
                time.sleep(1)
                dig.display(disstr)

            time.sleep(0.5)

    except Exception as ex:
        print("%s" % ex)


def Test_NRF24L01():
    nrf = NRF24L01(spi=2, csn='Y5', ce='Y4', payload_size=8)

    def recv():
        while True:
            s = nrf.slave()
            print(">> %s %s" % (s, get_current_time()))

    _thread.start_new_thread(recv, ())

    nrf2 = NRF24L01(spi=1, csn='X5', ce='X4', payload_size=8)
    i = 0
    while True:
        i += 1
        nrf2.master(i)
        print("<< %s %s" % (i, get_current_time()))
        time.sleep(3)

    while True:
        pass


def Test_NRF24L01_2():
    nrf = NRF24L01_2(spi=2, csn='Y5', ce='Y4', payload_size=16)

    def recv():
        nrf.set_rx_mode()
        while True:
            # s = nrf.slave()
            s = nrf.recv_data()
            print(">> %s %s" % (s, get_current_time()))

    _thread.start_new_thread(recv, ())

    nrf2 = NRF24L01_2(spi=1, csn='X5', ce='X4', payload_size=16)
    i = 0
    nrf2.set_tx_mode()
    while True:
        i += 1
        print("<< %s %s" % (i, get_current_time()))
        # nrf2.master(i)
        nrf2.send_data(i)
        time.sleep(3)

    while True:
        pass


def get_current_time():
    current_time = utime.time()
    local_time = utime.localtime(current_time)
    formatted_time = "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(
        local_time[0], local_time[1], local_time[2],
        local_time[3], local_time[4], local_time[5]
    )
    return formatted_time


print('Testing...')
# Test_TOFSense_F()
# Test_NRF24L01()
Test_NRF24L01_2()
