# echo-client.py

import socket
from time import sleep

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 9999  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while True:
        s.sendall(b"3; 32768; 32768")
        data = s.recv(1024)
        print(f"Received {data!r}")
        sleep(1)
