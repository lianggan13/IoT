"""NRF24L01 driver for MicroPython
"""

from micropython import const
import ustruct as struct
from pyb import Pin, SPI
import utime

# nRF24L01+ registers
CONFIG = const(0x00)
EN_AA = const(0x01)
EN_RXADDR = const(0x02)
SETUP_AW = const(0x03)
SETUP_RETR = const(0x04)
RF_CH = const(0x05)
RF_SETUP = const(0x06)
STATUS = const(0x07)
RX_ADDR_P0 = const(0x0a)
TX_ADDR = const(0x10)
RX_PW_P0 = const(0x11)	 # 接收数据通道0有效数据宽度(1~32字节)
RX_PW_P1 = const(0x12)	 # 接收数据通道1有效数据宽度(1~32字节)
RX_PW_P2 = const(0x13)	 # 接收数据通道2有效数据宽度(1~32字节)
RX_PW_P3 = const(0x14)	 # 接收数据通道3有效数据宽度(1~32字节)
RX_PW_P4 = const(0x15)	 # 接收数据通道4有效数据宽度(1~32字节)
RX_PW_P5 = const(0x16)	 # 接收数据通道5有效数据宽度(1~32字节)
FIFO_STATUS = const(0x17)
DYNPD = const(0x1c)

# CONFIG register
EN_CRC = const(0x08)  # enable CRC
CRCO = const(0x04)  # CRC encoding scheme 0=1 byte, 1=2 bytes
PWR_UP = const(0x02)  # 1=power up, 0=power down
PRIM_RX = const(0x01)  # RX/TX control 0=PTX, 1=PRX

# RF_SETUP register
POWER_0 = const(0x00)  # -18 dBm
POWER_1 = const(0x02)  # -12 dBm
POWER_2 = const(0x04)  # -6 dBm
POWER_3 = const(0x06)  # 0 dBm
SPEED_1M = const(0x00)
SPEED_2M = const(0x08)
SPEED_250K = const(0x20)

# STATUS register
RX_DR = const(0x40)  # RX data ready write 1 to clear
TX_DS = const(0x20)  # TX data sent write 1 to clear
MAX_RT = const(0x10)  # max retransmits reached write 1 to clear

# FIFO_STATUS register
RX_EMPTY = const(0x01)  # 1 if RX FIFO is empty

# constants for instructions
R_RX_PL_WID = const(0x60)  # read RX payload width
R_RX_PAYLOAD = const(0x61)  # read RX payload
W_TX_PAYLOAD = const(0xa0)  # write TX payload
FLUSH_TX = const(0xe1)  # flush TX FIFO
FLUSH_RX = const(0xe2)  # flush RX FIFO
NOP = const(0xff)  # use to read STATUS register

# pipes = (b'\xf0\xf0\xf0\xf0\xe1', b'\xf0\xf0\xf0\xf0\xd2')
pipes = (b'\xa5\xa5\xa5\xa5\xa5', b'\xa5\xa5\xa5\xa5\xa5')


class NRF24L01_2:
    def __init__(self, spi, csn, ce, channel=0, payload_size=16):
        assert payload_size <= 32

        self.buf = bytearray(1)

        # store the pins
        self.spi = SPI(spi)
        self.csn = Pin(csn, mode=Pin.OUT, value=1)
        self.ce = Pin(ce, mode=Pin.OUT, value=0)

        # init the SPI bus and pins
        self.init_spi(4000000)

        # reset everything
        self.ce.init(self.ce.OUT, value=0)
        self.csn.init(self.csn.OUT, value=1)

        self.payload_size = payload_size
        self.pipe0_read_addr = None
        utime.sleep_ms(5)

        # set address width to 5 bytes and check for device present
        self.reg_write(SETUP_AW, 0b11)
        if self.reg_read(SETUP_AW) != 0b11:
            raise OSError("nRF24L01+ Hardware not responding")

        # disable dynamic payloads 禁用动态载荷
        # 禁用动态载荷后，数据包的大小将由RX_PW_P0寄存器（或其他接收管道的相应寄存器）中的设置确定，发送方发送的数据包将严格按照这个大小发送
        self.reg_write(DYNPD, 0)

        # auto retransmit delay: 1750us  设置自动重发间隔时间:1750us
        # auto retransmit count: 8 最大自动重发次数:8次
        self.reg_write(SETUP_RETR, (6 << 4) | 8)

        # set rf power and speed 设置TX发射参数,0db增益,250K,低噪声增益开启
        # Best for point to point links
        self.set_power_speed(POWER_3, SPEED_250K)

        # init CRC  Reg: CRCO EN_CRC
        self.set_crc(2)

        # clear status flags
        self.reg_write(STATUS, RX_DR | TX_DS | MAX_RT)

        # set channel Reg:RF_CH
        self.set_channel(channel)

        # flush buffers
        self.flush_rx()
        self.flush_tx()

    def NRF24L01_RxPacket(self):
        # self.open_tx_pipe(pipes[1])
        # self.open_rx_pipe(1, pipes[0])
        self.start_listening()
        while True:
            if self.any():
                while self.any():
                    buf = self.recv()  # 接收内容
                    data, a = struct.unpack('ii', buf)  # 解析包unpack为解析包函数
                break
        return data

    def Send_Buf(self, buf):
        self.ce(0)
        self.reg_write(CONFIG, 0x0e)  # 发射模式
        self.ce(1)
        utime.sleep_us(15)
        self.NRF24L01_TxPacket(buf)
        self.ce(0)
        self.reg_write(CONFIG, 0x0e)  # 接收模式
        self.ce(1)

    def NRF24L01_TxPacket(self, buf, timeout=500):
        # power up
        # self.reg_write(CONFIG, (self.reg_read(CONFIG) | PWR_UP) & ~PRIM_RX)
        # utime.sleep_us(150)
        # send the data
        self.ce(0)
        self.spi.readinto(self.buf, W_TX_PAYLOAD)
        self.spi.write(buf)
        if len(buf) < self.payload_size:
            # pad out data
            self.spi.write(b'\x00' * (self.payload_size - len(buf)))
        self.ce(1)

        # enable the chip so it can send the data
        self.ce(1)
        utime.sleep_us(15)  # needs to be >10us
        self.ce(0)

        start = utime.ticks_ms()
        result = None
        while result is None and utime.ticks_diff(utime.ticks_ms(), start) < timeout:
            if not (self.reg_read(STATUS) & (TX_DS | MAX_RT)):
                break  # tx not finished

            # either finished or failed: get and clear status flags, power down 清除TX_DS或MAX_RT中断标志
            status = self.reg_write(STATUS, RX_DR | TX_DS | MAX_RT)
            self.reg_write(CONFIG, self.reg_read(CONFIG) & ~PWR_UP)
            result = 1 if status & TX_DS else 2

        if result == 2:
            print("send failed")

        self.flush_tx()
        self.flush_rx()

    def init_spi(self, baudrate):
        # SPI.init(baudrate=1000000, *, polarity=0, phase=0, bits=8, firstbit=SPI.MSB, sck=None, mosi=None, miso=None, pins=(SCK, MOSI, MISO))
        try:
            master = self.spi.MASTER
        except AttributeError as ex:
            print("%s" % ex)
            self.spi.init(baudrate=baudrate, polarity=0, phase=0)
        else:
            self.spi.init(master, baudrate=baudrate, polarity=0, phase=0)
            print("%s" % master)

    def reg_read(self, reg):
        self.csn(0)
        self.spi.readinto(self.buf, reg)
        self.spi.readinto(self.buf)
        self.csn(1)
        return self.buf[0]

    def reg_write_bytes(self, reg, buf):
        self.csn(0)
        self.spi.readinto(self.buf, 0x20 | reg)
        self.spi.write(buf)
        self.csn(1)
        return self.buf[0]

    def reg_write(self, reg, value: bytes):
        self.csn(0)
        self.spi.readinto(self.buf, 0x20 | reg)
        ret = self.buf[0]
        self.spi.readinto(self.buf, value)
        self.csn(1)
        return ret

    def flush_rx(self):
        self.csn(0)
        self.spi.readinto(self.buf, FLUSH_RX)
        self.csn(1)

    def flush_tx(self):
        self.csn(0)
        self.spi.readinto(self.buf, FLUSH_TX)
        self.csn(1)

    # power is one of POWER_x defines speed is one of SPEED_x defines
    def set_power_speed(self, power, speed):
        setup = self.reg_read(RF_SETUP) & 0b11010001
        self.reg_write(RF_SETUP, setup | power | speed)

    # length in bytes: 0, 1 or 2
    def set_crc(self, length):
        config = self.reg_read(CONFIG) & ~(CRCO | EN_CRC)
        if length == 0:
            pass
        elif length == 1:
            config |= EN_CRC
        else:
            config |= EN_CRC | CRCO
        self.reg_write(CONFIG, config)

    def set_channel(self, channel):
        self.reg_write(RF_CH, min(channel, 125))

    # address should be a bytes object 5 bytes long
    def open_tx_pipe(self, address):
        assert len(address) == 5
        self.reg_write_bytes(RX_ADDR_P0, address)
        self.reg_write_bytes(TX_ADDR, address)
        self.reg_write(RX_PW_P0, self.payload_size)

    # address should be a bytes object 5 bytes long
    # pipe 0 and 1 have 5 byte address
    # pipes 2-5 use same 4 most-significant bytes as pipe 1, plus 1 extra byte
    def open_rx_pipe(self, pipe_id, address):
        assert len(address) == 5
        assert 0 <= pipe_id <= 5
        if pipe_id == 0:
            self.pipe0_read_addr = address
        if pipe_id < 2:
            self.reg_write_bytes(RX_ADDR_P0 + pipe_id, address)
        else:
            self.reg_write(RX_ADDR_P0 + pipe_id, address[0])
        self.reg_write(RX_PW_P0 + pipe_id, self.payload_size)
        self.reg_write(EN_RXADDR, self.reg_read(EN_RXADDR) | (1 << pipe_id))

    def start_listening(self):
        self.reg_write(CONFIG, self.reg_read(CONFIG) | PWR_UP | PRIM_RX)
        self.reg_write(STATUS, RX_DR | TX_DS | MAX_RT)

        if self.pipe0_read_addr is not None:
            self.reg_write_bytes(RX_ADDR_P0, self.pipe0_read_addr)

        self.flush_rx()
        self.flush_tx()
        self.ce(1)
        utime.sleep_us(130)

    def stop_listening(self):
        self.ce(0)
        self.flush_tx()
        self.flush_rx()

    # returns True if any data available to recv
    def any(self):
        return not bool(self.reg_read(FIFO_STATUS) & RX_EMPTY)

    def recv(self):
        # get the data
        self.csn(0)
        self.spi.readinto(self.buf, R_RX_PAYLOAD)
        buf = self.spi.read(self.payload_size)
        self.csn(1)
        # clear RX ready flag
        self.reg_write(STATUS, RX_DR)

        return buf

    # blocking wait for tx complete
    def send(self, buf, timeout=500):
        self.send_start(buf)
        start = utime.ticks_ms()
        result = None
        while result is None and utime.ticks_diff(utime.ticks_ms(), start) < timeout:
            result = self.send_done()  # 1 == success, 2 == fail
        if result == 2:
            raise OSError("send failed")

    # non-blocking tx
    def send_start(self, buf):
        # power up
        self.reg_write(CONFIG, (self.reg_read(CONFIG) | PWR_UP) & ~PRIM_RX)
        utime.sleep_us(150)
        # send the data
        self.csn(0)
        self.spi.readinto(self.buf, W_TX_PAYLOAD)
        self.spi.write(buf)
        if len(buf) < self.payload_size:
            # pad out data
            self.spi.write(b'\x00' * (self.payload_size - len(buf)))
        self.csn(1)

        # enable the chip so it can send the data
        self.ce(1)
        utime.sleep_us(15)  # needs to be >10us
        self.ce(0)

    # returns None if send still in progress, 1 for success, 2 for fail
    def send_done(self):
        if not (self.reg_read(STATUS) & (TX_DS | MAX_RT)):
            return None  # tx not finished

        # either finished or failed: get and clear status flags, power down
        status = self.reg_write(STATUS, RX_DR | TX_DS | MAX_RT)
        self.reg_write(CONFIG, self.reg_read(CONFIG) & ~PWR_UP)
        return 1 if status & TX_DS else 2

    def master(self, data):
        self.open_tx_pipe(pipes[0])
        self.open_rx_pipe(1, pipes[1])
        self.stop_listening()
        try:
            self.send(struct.pack('i', data))  # 发送内容 pack为封包函数
            self.start_listening()
            start_time = utime.ticks_ms()
            timeout = False
            while not self.any() and not timeout:
                if utime.ticks_diff(utime.ticks_ms(), start_time) > 250:
                    timeout = True
        except OSError:
            pass

    def slave(self):
        self.open_tx_pipe(pipes[1])
        self.open_rx_pipe(1, pipes[0])
        self.start_listening()
        while True:
            if self.any():
                while self.any():
                    buf = self.recv()  # 接收内容
                    data, a = struct.unpack('ii', buf)  # 解析包unpack为解析包函数
                break
        return data