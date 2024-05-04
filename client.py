from enum import Enum
import argparse
import socket
import sys


class client:
    def __init__(self):
        client.user = ""

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum):
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1

    # ******************** METHODS *******************

    def register(self, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'REGISTER\0'
            print("sending comand")
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            print("command sent")
            response_code = sock.recv(1).decode('utf-8')
            print("response code is ", response_code)
            if response_code == '0':
                print("c> REGISTER OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> USERNAME IN USE")
                self.user = ""
                return client.RC.USER_ERROR
            else:
                print("c> REGISTER FAIL")
                self.user = ""
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def unregister(self, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'UNREGISTER\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')
            print("response code is ", response_code)
            if response_code == '0':
                print("c> UNREGISTER OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            else:
                print("c> UNREGISTER FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def connect(self, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'CONNECT\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')
            print("response code %s", response_code)
            
            if response_code == '0':
                print("c> CONNECT OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> CONNECT FAIL , USER DOES NOT EXIST")
                self.user = ""
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> USER ALREADY CONNECTED")
                self.user = ""
                return client.RC.USER_ERROR
            else:
                print("c> CONNECT FAIL")
                self.user = ""
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def disconnect(self, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar nombre al servidor y el comando
            command = 'DISCONNECT\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')

            if response_code == '0':
                print("c> DISCONNECT OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> DISCONNECT FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> DISCONNECT FAIL / USER NOT CONNECTED")
                return client.RC.USER_ERROR
            else:
                print("c> DISCONNECT FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def publish(self, fileName, description, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar nombre al servidor y el comando
            command = 'PUBLISH\0'
            sock.sendall(command.encode('utf-8'))
            #Enviar user
            file_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(file_bytes)
            #Enviar Filename
            file_bytes = bytes(fileName + '\0', 'utf8')
            sock.sendall(file_bytes)
            #Enviar descripción
            file_bytes = bytes(description + '\0', 'utf8')
            sock.sendall(file_bytes)

            response_code = sock.recv(1).decode('utf-8')

            print("response code is", response_code)

            if response_code == '0':
                print("c> PUBLISH OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> PUBLISH FAIL , USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> PUBLISH FAIL , USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif response_code == '3':
                print("c> PUBLISH FAIL , CONTENT ALREADY PUBLISHED")
                return client.RC.USER_ERROR
            else:
                print("c> PUBLISH FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def delete(self, fileName, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar nombre al servidor y el comando
            command = 'DELETE\0'
            sock.sendall(command.encode('utf-8'))
            file_bytes = bytes(fileName + '\0', 'utf8')
            sock.sendall(file_bytes)
            response_code = sock.recv(1).decode('utf-8')

            if response_code == '0':
                print("c> DELETE OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> DELETE FAIL , USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> DELETE FAIL , USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif response_code == '3':
                print("c> DELETE FAIL , CONTENT NOT PUBLISHED")
                return client.RC.USER_ERROR
            else:
                print("c> DELETE FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    def listusers(self, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR

            # Enviar nombre al servidor y el comando
            command = 'LIST_USERS\0'
            sock.sendall(command.encode('utf-8'))
            sock.sendall(self.user.encode('utf-8') + b'\0')  # enviar el usuario solicitante
            response_code = sock.recv(1).decode('utf-8')

            if response_code == '0':
                # Recibir el número de usuarios
                number_of_users = ""
                char = sock.recv(1).decode('utf-8')
                while char != '\0':
                    number_of_users += char
                    char = sock.recv(1).decode('utf-8')
                number_of_users = int(number_of_users)

                print("c> LIST_USERS OK")
                print(f"Number of connected users: {number_of_users}")

                # Recibir la información de cada usuario conectado
                for _ in range(number_of_users):
                    user_info = ""
                    char = sock.recv(1).decode('utf-8')
                    while char != '\0':
                        user_info += char
                        char = sock.recv(1).decode('utf-8')
                    print(user_info)

                return client.RC.OK
            elif response_code == '1':
                print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> LIST_USERS FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            else:
                print("no se puede abrir")
                print("c> LIST_USERS FAIL")
                return client.RC.ERROR
        except Exception as e:
            print(f"c> LIST_USERS EXCEPTION: {e}")
            return client.RC.ERROR


    @staticmethod
    def listcontent(user):
        #  Write your code here
        return client.RC.ERROR

    def getfile(self, remote_FileName, local_FileName, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'GET_FILE\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            file_bytes = bytes(remote_FileName + local_FileName + '\0', 'utf8')
            sock.sendall(file_bytes)
            response_code = sock.recv(1).decode('utf-8')

            if response_code == '0':
                print("c> GET_FILE OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> GET_FILE FAIL / FILE NOT EXIST")
                return client.RC.USER_ERROR
            else:
                print("c> GET_FILE FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR

    # * @brief Command interpreter for the client. It calls the protocol functions.
    def shell(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.connect((self._server, self._port))

            while True:
                try:
                    command = input("c> ")
                    line = command.strip().split()
                    if not line:
                        continue

                    action = line[0].upper()
                    if action == "REGISTER" and len(line) == 2:
                        self.user = line[1]
                        self.register(sock)
                    elif action == "UNREGISTER" and len(line) == 2:
                        self.user = line[1]
                        self.unregister(sock)
                    elif action == "CONNECT" and len(line) == 2:
                        self.user = line[1]
                        self.connect(sock)
                    elif action == "PUBLISH" and len(line) >= 3:
                        description = ' '.join(line[2:])
                        self.publish(line[1], description, sock)
                    elif action == "DELETE" and len(line) == 2:
                        self.delete(line[1], sock)
                    elif action == "LIST_USERS" and len(line) == 1:
                        self.listusers(sock)
                    elif action == "LIST_CONTENT" and len(line) == 2:
                        self.listcontent(line[1])
                    elif action == "DISCONNECT" and len(line) == 2:
                        self.user = line[1]
                        self.disconnect(sock)
                    elif action == "GET_FILE" and len(line) == 4:
                        self.getfile(line[1], line[2], line[3], sock)
                    elif action == "QUIT" and len(line) == 1:
                        break
                    else:
                        print("Error: command not valid or incorrect syntax.")
                except Exception as e:
                    print("Exception:", str(e))
        finally:
            # Close the socket before exiting the shell
            sock.close()
            print("Session ended.")

    # * @brief Prints program usage
    @staticmethod
    def usage():
        print("Usage: python3 client.py -s <server> -p <port>")

    # * @brief Parses program execution arguments
    @staticmethod
    def parseArguments(argv):
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535")

        client._server = args.s
        client._port = args.p
        return True

    # ******************** MAIN *********************
    def main(self, argv):
        if (not client.parseArguments(argv)):
            client.usage()
            return

        self.shell()
        print("+++ FINISHED +++")


if __name__ == "__main__":
    client().main([])