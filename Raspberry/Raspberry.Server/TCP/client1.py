from socketcom import *
from builtins import print
from threading import Thread
import sys,time,_thread
import datetime
import socket
import asyncio

api_btn_state = "/api/btn/state"

def handle_send(client):
    while True:
        if(client._isconnected == False):
            break
        else:
            client.send_data(api_btn_state)
        time.sleep(2)

def handle_client_connected(sender:TcpClient):
    print(f"√ client [{sender._ip}:{sender._port}] connected.")
    # start_send
    _thread.start_new_thread(handle_send,(sender,))

def handle_client_disconnected(sender:TcpClient):
    print(f"× client [{sender._ip}:{sender._port}] diconnected.")
    # _thread.exit()

def handle_client_received(sender:TcpClient,msg:str):
    print("Received: %s len: %s %s" % (msg, str(len(msg)),
                        datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))

def handle_exception(sender:TcpServer,exp:str):
    print (exp)


async def main():
    # client = TcpClient("127.0.0.1",1918)
    client = TcpClient("192.168.0.2",1918)
    client.connected_event += handle_client_connected
    client.disconnected_event += handle_client_disconnected
    client.received_event += handle_client_received
    client.exception_event += handle_exception
    client.start()

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
    asyncio.get_event_loop().run_forever()




