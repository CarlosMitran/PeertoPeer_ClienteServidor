from enum import Enum
import argparse
import socket
import sys
import threading
import zeep


class client:
    def __init__(self):
        client.user = ""
        self.connected = False

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
    def quit(self):
        #Prepara las cadenas que se deben enviar al servidor para cerrar el cliente

        if not self.connected:
            print("c> QUIT FAIL / CONNECT FIRST")
            return client.RC.USER_ERROR
        try:
            #Conexión con el socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = 'QUIT\0'
            sock.sendall(command.encode('utf-8'))
            file_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(file_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)
            #Código de respuesta y cierre de conexión
            response_code = sock.recv(2).decode('utf-8')
            sock.close()
            if response_code == '0':
                print("c> QUIT OK")
                self.connected = False 
                return client.RC.OK
            elif response_code == '1': 
                print("c> QUIT FAIL")
                return client.RC.ERROR
            else: 
                print("c> Q")
                return client.RC.USER_ERROR
        except:
            sock.close()
            return client.RC.ERROR
        finally:
            sock.close()
            sys.exit(0)


    def register(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = 'REGISTER\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)

            # Recibir respuesta del servidor y cerrar conexión

            response_code = sock.recv(1).decode('utf-8')
            sock.close()
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
            sock.close()
            return client.RC.ERROR

    def unregister(self):
        #Prepara las cadenas que se deben enviar al servidor para eliminar del registro al usuario
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = 'UNREGISTER\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)
            # Recibir respuesta del servidor y cerrar conexión

            response_code = sock.recv(1).decode('utf-8')
            sock.close()
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
            sock.close()
            return client.RC.ERROR

    def connect(self):
        #Prepara las cadenas que se deben enviar al servidor para la conexión del usuario
        try:
            #Conexión al socket del servidor
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = "CONNECT\n"
            sock.sendall(command.encode('utf-8'))

            user_bytes = (self.user + '\n').encode('utf-8')
            sock.sendall(user_bytes)

            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)

            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)

            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)

            # Recibir respuesta del servidor y cerrar conexión
            response_code = sock.recv(1).decode('utf-8').strip()
            sock.close()

            if response_code == '0':
                print("c> CONNECT OK")
                self.connected = True
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
            print("Error al conectar al usuario")
            sock.close()
            return client.RC.ERROR

    def disconnect(self):
        #Prepara las cadenas que se deben enviar al servidor para la desconexión del usuario
        try:
            #Conexión al socket del servidor
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = 'DISCONNECT\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)
            #Recibir respuesta y cerrar conexión
            response_code = sock.recv(2).decode('utf-8')
            sock.close()
            print("response code is ", response_code)
            if response_code == '0':
                print("c> DISCONNECT OK")
                self.connected = False
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
            sock.close()
            return client.RC.ERROR

    def publish(self, fileName, description):
        #Prepara las cadenas que se deben enviar al servidor para publicar nombre y descripción de fichero.
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
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
            fecha = self.devolverFecha()
            #Envío de fecha
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)

            #Recibir respuesta y cerrar conexión
            response_code = sock.recv(1).decode('utf-8')
            sock.close()
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
            sock.close()
            return client.RC.ERROR

    def delete(self, fileName):
        #Prepara las cadenas que se deben enviar al servidor para borrar el usuario.
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            # Enviar todos los datos.
            command = 'DELETE\0'
            sock.sendall(command.encode('utf-8'))
            user_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(fileName + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)
            #Recibir respuesta y cerrar conexión
            response_code = sock.recv(3).decode('utf-8')
            sock.close()
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
            sock.close()
            return client.RC.ERROR

    def listusers(self):
        #Prepara el envío del comando listusers y se encarga de recibir y imprimir los resultados
        counter = 0
        finalmessage = ""
        number_of_users = 0
        response_code = -1
        end = 0
        try:
            #Conexión al socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR

            # Enviar todos los datos. Fichero y descripción deben enviarse vacíos para que el servidor no tenga problemas.
            command = 'LIST_USERS\0'
            sock.sendall(command.encode('utf-8'))
            file_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(file_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)

            while True:
                msg = sock.recv(256).decode('utf-8')
                msgcpy = msg
                totalmsg = [line for line in msgcpy.split('\n') if line.strip()]
                for i in totalmsg:
                    if i[0] == "@":
                        if len(i) > 2 and i[0:3] == "@IP":
                            ip = i[3:]
                        elif len(i) > 4 and i[0:5] == "@PORT":
                            port = i[5:]
                            finalmessage = finalmessage + str(user) + " " + str(ip) + " " + str(port) + '\n'
                            number_of_users += 1
                        else:
                            user = i[1:]
                    else:
                        response_code = i
                        end = 1
                        break

                if '\0' in msg:
                    response_code = msg[-1]
                    break
                if end == 1:
                    break

        except Exception as e:
            sock.close()
            print(f"c> LIST_USERS EXCEPTION: {e}")
            return client.RC.ERROR
        sock.close()
        if response_code == '0':
            # Recibir el número de usuarios
            print("c> LIST_USERS OK")
            print(f"Number of connected users: {number_of_users}\n")
            # Recibir la información de cada usuario conectado
            print(finalmessage)
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

    def listcontent(self, username):
        #Envío de los datos al servidor y conexión con este.
        counter = 0
        finalmessage = ""
        number_of_users = 0
        response_code = -1
        end = 0
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._server, self._port))
            if self.user == "":
                print("c> DISCONNECT FAIL / CONNECT FIRST")
                return client.RC.USER_ERROR
            #Un envío por cada parámetro, comando, usuario, filename, descripción, fecha
            command = 'LIST_CONTENT\0'
            sock.sendall(command.encode('utf-8'))
            file_bytes = bytes(self.user + '\0', 'utf8')
            sock.sendall(file_bytes)
            user_bytes = bytes(username + '\0', 'utf8')
            sock.sendall(user_bytes)
            user_bytes = bytes(" "'\0', 'utf8')
            sock.sendall(user_bytes)
            fecha = self.devolverFecha()
            fecha_bytes = bytes(fecha + '\0', 'utf8')
            sock.sendall(fecha_bytes)
            #Mientras siga recibiendo datos, añade a una lista estos, tras ello, se imprime todo
            while True:
                msg = sock.recv(256).decode('utf-8')
                msgcpy = msg
                totalmsg = [line for line in msgcpy.split('\n') if line.strip()]
                totalmsg = [element for element in totalmsg if element != "@"]
                for i in totalmsg:
                    #El servidor envía al principio de las cadenas un identificador. Este identificador sirve para poder
                    #saber dónde va cada parte de la cadena a la hora de imprimir
                    if i[0] == "@" and counter == 0:
                        filename = i[1:]
                        counter = 1
                    elif i[0] == "@" and counter == 1:
                        counter = 0
                        descriptioname = i[1:]
                        finalmessage = finalmessage + filename + " " + descriptioname + " " + "\n"
                    else:
                        response_code = i
                        end = 1
                        break

                if '\0' in msg:
                    response_code = msg[-1]
                    break
                if end == 1:
                    break

        except Exception as e:
            print(f"c> LIST_USERS EXCEPTION: {e}")
            sock.close()
            return client.RC.ERROR
        sock.close()
        if response_code == '0':
            # Recibir el número de usuarios
            print("c> LIST_CONTENT OK")
            # Recibir la información de cada usuario conectado
            print(finalmessage)
            return client.RC.OK
        elif response_code == '1':
            print("c> LIST_USERS FAIL, USER DOES NOT EXIST")
            return client.RC.USER_ERROR
        elif response_code == '2':
            print("c> LIST_USERS FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        else:
            print("c> LIST_USERS FAIL")
            return client.RC.ERROR

    def getfile(self, server, port, remote_FileName, local_FileName, ):
        #Se conecta al socket del servidor de otro cliente python, envía cadena del file que quiere, y lo recibe
        server_address = (server, int(port))
        print('conectando a {} y puerto {}'.format(*server_address))
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(server_address)
        try:
            # Enviar nombre al servidor y el comando
            message = bytes(remote_FileName + '\0', 'utf8')
            sock.sendall(message)
            filename = local_FileName
            fo = open(filename, "w")
            #Recibir el fichero
            while True:
                msg = sock.recv(1024).decode('utf-8')
                recv_len = len(msg)
                if recv_len < 1024:
                    fo.write(msg)
                    break
                fo.write(msg)
                if '\0' in msg:
                    break
            #Valor de respuesta
            response_code = sock.recv(1).decode('utf-8')
            sock.close()
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
            sock.close()
            return client.RC.ERROR

    # * @brief Command interpreter for the client. It calls the protocol functions.
    def shell(self):
        #Función shell, se encarga de imprimir las cadenas necesarias y de leer el input del usuario. El socket se inicia
        #en cada función de manera separada, para poder conectarse a sockets de otros clientes o al socket servidor dependiendo
        #de lo que se necesite
        try:
            while True:
                try:
                    command = input("c> ")
                    line = command.strip().split()
                    if not line:
                        continue
                    action = line[0].upper()
                    if action == "REGISTER" and len(line) == 2:
                        self.user = line[1]
                        self.register()
                    elif action == "UNREGISTER" and len(line) == 2:
                        self.user = line[1]
                        self.unregister()
                    elif action == "CONNECT" and len(line) == 2:
                        self.user = line[1]
                        self.connect()
                    elif action == "PUBLISH" and len(line) >= 3:
                        description = ' '.join(line[2:])
                        self.publish(line[1], description, )
                    elif action == "DELETE" and len(line) == 2:
                        self.delete(line[1])
                    elif action == "LIST_USERS" and len(line) == 1:
                        self.listusers()
                    elif action == "LIST_CONTENT" and len(line) == 2:
                        self.listcontent(line[1])
                    elif action == "DISCONNECT" and len(line) == 2:
                        self.user = line[1]
                        self.disconnect()
                    elif action == "GET_FILE" and len(line) == 5:
                        self.getfile(line[1], line[2], line[3], line[4])
                    elif action == "QUIT" and len(line) == 1:
                        self.quit()
                    else:
                        print("Error: command not valid or incorrect syntax.")
                except Exception as e:
                    print("Exception:", str(e))

        finally:
            # Close the socket before exiting the shell
            print("Session ended.")
            sys.exit(0)

    def init_server(self):
        #Inicialización del servidor del cliente python, el cliente escucha y envía los ficheros cuando recibe una petición
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        #Puerto por defecto, 100 + puerto cliente
        server_address = (self._server, self._port + 100)
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

    def devolverFecha(self):
        #Conexión con el websocket y devolución del valor de fecha
        wsdl_url = "http://localhost:8100/?wsdl"
        soap = zeep.Client(wsdl=wsdl_url)
        result = soap.service.fecha()
        return result

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
        #Se busca si los argumentos son correctos, si lo son, se inician los hilos de servidor y cliente del cliente python
        if (not client.parseArguments(argv)):
            client.usage()
            return
        server_thread = threading.Thread(target=self.init_server)
        client_thread = threading.Thread(target=self.shell)

        server_thread.start()
        client_thread.start()

        client_thread.join()
        server_thread.join()
        print("+++ FINISHED +++")


if __name__ == "__main__":
    client().main([])
