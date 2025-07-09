# tcp_server.py
import socket

HOST = '0.0.0.0'   # استقبل من أي IP
PORT = 9000        # نفس البورت اللي جهاز DF703 بيبعته عليه

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening on {HOST}:{PORT} ...")
    while True:
        conn, addr = s.accept()
        print(f"Connected by {addr}")
        with conn:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                print(f"Received: {data}")
