#!/usr/bin/env python
import signal
import sys

# Thread aus liste entfernen wenn ende

import pyvjoy
import socket
import threading

buttonsPerClient = 3
axisPerClient = 2

j = pyvjoy.VJoyDevice(1)


def setAxis(axis, value):
    match axis:
        case 1:
            j.data.wAxisX = value
        case 2:
            j.data.wAxisY = value
        case 3:
            j.data.wAxisZ = value
        case 4:
            j.data.wAxisXRot = value
        case 5:
            j.data.wAxisYRot = value
        case 6:
            j.data.wAxisZRot = value
        case 7:
            j.data.wSlider = value
        case 8:
            j.data.wDial = value
        case _:
            print("ERROR: Unknown axis: ", axis)


class ClientThread(threading.Thread):

    def __init__(self, ip, port, socket, status, offsetB, offsetA):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.socket = socket
        self.status = status
        self.offsetB = offsetB
        self.offsetA = offsetA
        self.running = True
        print("[+] New thread started for " + ip + ":" + str(port))

    def run(self):
        print("Connection from : " + ip + ":" + str(port))

        self.socket.send(b"\nWelcome to the server\n\n")

        data = "not empty"

        while len(data) & self.running:
            data = self.socket.recv(2048)
            split = data.split(b";")
            buttons = int(split[0])
            if buttons >= pow(2, buttonsPerClient):
                continue
            print("Client sent : " + str(data))
            j.data.lButtons = j.data.lButtons & ~((pow(2, buttonsPerClient) - 1) << self.offsetB)  # reset
            j.data.lButtons = j.data.lButtons | (buttons << self.offsetB)  # set

            for i in range(1, axisPerClient + 1):
                setAxis(i + self.offsetA, int(split[i]))
            j.update()
            self.socket.send(b"Server received : " + data)

        print("Client disconnected...")
        threads.remove(self)


host = "127.0.0.1"
port = 9999

tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

tcpsock.bind((host, port))
threads = []


def signal_handler(signal, frame):
    print("\nShutting down server...")
    for thread in threads:
        thread.running = False
        thread.join()
    tcpsock.close()
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

tcpsock.settimeout(3)

print("\nListening for incoming connections...")
while True:
    try:
        tcpsock.listen(4)
        (clientsock, (ip, port)) = tcpsock.accept()
        newthread = ClientThread(ip, port, clientsock, False, len(threads) * buttonsPerClient,
                                 len(threads) * axisPerClient)
        newthread.start()
        threads.append(newthread)
    except socket.timeout:
        pass
