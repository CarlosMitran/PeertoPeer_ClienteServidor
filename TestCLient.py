import socket
import sys


def main(argv):
    arguments = len(sys.argv)
    if arguments < 5:
        print('Uso: client_base_tcp  <host> <port>')
        exit()

    server_address = (sys.argv[1], int(sys.argv[2]))
    print('conectando a {} y puerto {}'.format(*server_address))
    sock.connect(server_address)

    try:

        message = bytes(sys.argv[3] + '\0', 'utf8')
        sock.sendall(message)
        filename = sys.argv[4]
        fo = open(filename, "w")
        while True:
            msg = sock.recv(1024).decode('utf-8')
            recv_len = len(msg)
            if recv_len < 1024:
                fo.write(msg)
                break
            fo.write(msg)
            if '\0' in msg:
                break
    finally:
        sock.close()


if __name__ == "__main__":
    main([])
