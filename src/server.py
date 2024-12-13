import os
os.environ['SSLKEYLOGFILE'] = 'sslkeylogfile.txt'

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--cert', required=True)
parser.add_argument('--key', required=True)
args = parser.parse_args()

import socket
import ssl

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('localhost', 8888))
server_socket.listen(5)

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain(certfile=args.cert, keyfile=args.key)

while True:
    client_socket, addr = server_socket.accept()
    print(f"Connection from {addr}")

    ssl_socket = context.wrap_socket(client_socket, server_side=True)

    try:
        while True:
            message = ssl_socket.recv(1024).decode('utf-8')
            if message == 'exit':
                print("Client disconnected")
                break
            else:
                print(f"Received: {message}")

            ssl_socket.send(input("Server: ").encode('utf-8'))

    finally:
        ssl_socket.shutdown(socket.SHUT_RDWR)
        ssl_socket.close()
