import os
os.environ['SSLKEYLOGFILE'] = 'sslkeylogfile.txt'

import argparse
parser = argparse.ArgumentParser(description='Run a TLS client.')
parser.add_argument('--cert', required=True, help='Path to the server certificate file')
args = parser.parse_args()

import socket
import ssl

context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH)
context.load_verify_locations(args.cert)

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

ssl_socket = context.wrap_socket(client_socket, server_hostname='localhost')

ssl_socket.connect(('localhost', 8888))

print("Connected to server. Type 'exit' to disconnect.")

try:
    while True:
        message = input("Client: ")
        ssl_socket.send(message.encode('utf-8'))

        if message.lower() == 'exit':
            break

        print(f'Received: {ssl_socket.recv(1024).decode("utf-8")}')
finally:
    ssl_socket.shutdown(socket.SHUT_RDWR)
    ssl_socket.close()
