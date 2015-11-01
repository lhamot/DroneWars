import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer
from gen_py.thrift.ttypes import *

def getClient(adress, port):
    transport = thrift.transport.TSocket.TSocket(adress, port)
    transport.open()
    protocol = thrift.protocol.TBinaryProtocol.TBinaryProtocol(transport)
    service = gen_py.thrift.EngineServer.Client(protocol)

    #service.createUniverse(False)
    return service