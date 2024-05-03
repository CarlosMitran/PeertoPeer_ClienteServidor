from enum import Enum
import argparse
import socket
import os

def init_server(server, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = (server, port)
    sock.bind(server_address)
    sock.listen(5)

    while True:
        connection, client_address = sock.accept()
        try:
            message = ''
            while True:
                msg = connection.recv(1)
                if msg == b'\0':
                    break
                message += msg.decode('utf8')

            with open(message, "r") as file:
                # Read the content of the file
                data = file.read()
                # Send the file data back to the client
                while data:
                    # Encode the file data into bytes using UTF-8 encoding
                    connection.sendall(data.encode('utf-8'))
                    data = file.read()
            connection.sendall(b"0")

        except FileNotFoundError:
            # Handle FileNotFoundError (file not found)
            print(f'File "{message}" not found.')

        finally:
            # Close the connection
            connection.close()


def parseArguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', type=str, required=True, help='Server IP')
    parser.add_argument('-p', type=int, required=True, help='Server Port')
    args = parser.parse_args()

    if args.s is None:
        parser.error("Usage: python3 client.py -s <server> -p <port>")

    if (args.p < 1024) or (args.p > 65535):
        parser.error("Error: Port must be in the range 1024 <= port <= 65535")

    server = args.s
    port = args.p
    return init_server(server, port)


def main(argv):
    parseArguments(argv)


if __name__ == "__main__":
    main([])
