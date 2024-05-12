import datetime
from spyne import Application, ServiceBase, Integer, Unicode, rpc, String
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication


class DatetimeServer(ServiceBase):
    #Servicio websocket que devuelve la fecha y la hora. Uso de soap y spyne para levantar un servidor que recibe y envía peticiones
    #Servicio RPC de Soap y la función que da la fecha
    @rpc(_returns=String)
    def fecha(ctx):
        current_datetime = datetime.datetime.now()
        formatted_datetime = current_datetime.strftime("%d/%m/%Y %H:%M:%S")
        return formatted_datetime
    #Protocolos
application = Application(
    services=[DatetimeServer],
    tns='http://tests.python-zeep.org/',
    in_protocol=Soap11(validator='lxml'),
    out_protocol=Soap11())

application = WsgiApplication(application)

if __name__ == '__main__':
    #Se inicia el servidor
    import logging

    from wsgiref.simple_server import make_server

    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)

    logging.info("listening to http://127.0.0.1:8100")
    logging.info("wsdl is at: http://localhost:8100/?wsdl")

    server = make_server('127.0.0.1', 8100, application)
    server.serve_forever()
