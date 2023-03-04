from socketcom import *
from threading import Thread
import sys,time,_thread
import datetime
import socket
import asyncio
import RPi.GPIO as GPIO


P_BUTTON = 20 # key button pin
api_btn_state = "/api/btn/state"
server = TcpServer("192.168.0.9",32769)

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(P_BUTTON, GPIO.IN, GPIO.PUD_UP)
    GPIO.add_event_detect(P_BUTTON,GPIO.FALLING,button_state_changed,200)

def button_state_changed(args):
    state = "Button pressed"
    data = f"Push current state: {state}"
    for h in list(server._clients.keys()): # dictionary changed size during iteration
        server.send_data(h,data)


def handle_client_connected(sender:TcpServer,host:str):
    print ("√ connected with client at %s" % host)

def handle_client_disconnected(sender:TcpServer,host:str):
    print ("× disConnected with client at %s" % host)

def handle_client_received(sender:TcpServer,host:str,msg:str):
    print(">> %s len: %s from [%s] %s" % (msg, str(len(msg)),host,
                        datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
    if msg[:-1]== api_btn_state:  # remove trailing "\0"
            if GPIO.input(P_BUTTON) == GPIO.LOW:
                data = "Button pressed"
            else:
                data = "Button released"
            sender.send_data(host,data)

def handle_exception(sender:TcpServer,exp:str):
    print (exp)


async def main():
    setup()
    server.connected_event += handle_client_connected
    server.disconnected_event += handle_client_disconnected
    server.received_event += handle_client_received
    server.exception_event += handle_exception
    await server.start()
    

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
    asyncio.get_event_loop().run_forever()