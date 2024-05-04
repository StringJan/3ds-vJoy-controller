#!/usr/bin/env python

# Thread aus liste entfernen wenn ende

import pyvjoy
import socket
import threading
import time
import re

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
        print("[+] New thread started for " + ip + ":" + str(port))

    def run(self):
        print("Connection from : " + ip + ":" + str(port))

        self.socket.send(b"\nWelcome to the server\n\n")

        data = "not empty"
        leftover = ""
        pattern = r'^<\d+;\s*\d+;\s*\d+>'

        while True:
            print("Waiting for data")
            data = self.socket.recv(25).decode("utf-8")
            print("Client sent:" + str(data))
            data = leftover + data
            print("Concatenated:" + str(data))
            # Check if packet contains d
            if "d" in data:
                break
            matches = re.findall(pattern, str(data))
            packet = ""
            if len(matches) > 0:
                packet = matches[0]
                print("Packet:" + str(packet))

            leftover = data.replace(packet, "")
            print("Leftover: " + str(leftover))
            if packet == "":
                print("No full packet received")
                continue
            curr_pack = packet.rstrip(">").lstrip("<")
            print("Current packet:" + curr_pack)
            try:
                split = curr_pack.split(";")
            except:
                print("Split Error")
            print("Processed:" + str(split))
            buttons = int(split[0])
            if buttons >= pow(2, buttonsPerClient):
                continue
            j.data.lButtons = j.data.lButtons & ~((pow(2, buttonsPerClient) - 1) << self.offsetB)  # reset
            j.data.lButtons = j.data.lButtons | (buttons << self.offsetB)  # set

            for i in range(1, axisPerClient+1):
                setAxis(i+self.offsetA, int(split[i]))
            j.update()
            time.sleep(0.1)
            self.socket.send(b"Ack")
            print("Sent Ack")

        print("Client disconnected...")
        threads.remove(self)


host = "0.0.0.0"
port = 9999

tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

tcpsock.bind((host, port))
threads = []

while True:
    tcpsock.listen()
    print("\nListening for incoming connections...")
    (clientsock, (ip, port)) = tcpsock.accept()
    newthread = ClientThread(ip, port, clientsock, False, len(threads)*buttonsPerClient, len(threads)*axisPerClient)
    newthread.start()
    threads.append(newthread)
