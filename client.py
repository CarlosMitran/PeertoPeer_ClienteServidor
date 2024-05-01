from enum import Enum
import argparse
import socket
import sys


class client :
    def __init__(self):
        client.user = ""


    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
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
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')

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
        finally:
            sock.close()


    def unregister(self, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'UNREGISTER\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')

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
        finally:
            sock.close()


    def connect(self, sock):
        try:
            # Enviar nombre al servidor y el comando
            command = 'CONNECT\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            response_code = sock.recv(1).decode('utf-8')

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
        finally:
            sock.close()

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
        finally:
            sock.close()

    def publish(self, fileName, description, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar nombre al servidor y el comando
            command = 'PUBLISH\0'
            sock.sendall(command.encode('utf-8'))
            file_bytes = bytes(fileName + description + '\0', 'utf8')
            sock.sendall(file_bytes)
            response_code = sock.recv(1).decode('utf-8')

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
        finally:
            sock.close()


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
        finally:
            sock.close()

    def listusers(self, sock):
        try:
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar nombre al servidor y el comando
            command = 'LIST_USERS\0'
            sock.sendall(command.encode('utf-8'))
            response_code = sock.recv(1).decode('utf-8')

            if response_code == '0':
                #FALTA CODIGO EN EL PRINT 
                print("c> LIST_USERS OK")
                return client.RC.OK
            elif response_code == '1':
                print("c> LIST_USERS FAIL , USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif response_code == '2':
                print("c> LIST_USERS FAIL , USER NOT CONNECTED")
                return client.RC.USER_ERROR
            else:
                print("c> LIST_USERS FAIL")
                return client.RC.ERROR
        except:
            return client.RC.ERROR
        finally:
            sock.close()

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
        finally:
            sock.close()

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    def shell(self,sock):

        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0] == "REGISTER"):
                        if (len(line) == 2):
                            self.user = line[1]
                            self.register(sock)
                            self.user = ""
                        else:
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1],sock)
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1],sock)
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")
                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1],sock)
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")

            except Exception as e:
                print("Exception: " + str(e))

    # * @brief Prints program usage
    @staticmethod
    def usage() :
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

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        args = (client._server, client._port)
        sock.connect(args)
        self.shell(sock)
        print("+++ FINISHED +++")


if __name__ == "__main__":
    client().main([])