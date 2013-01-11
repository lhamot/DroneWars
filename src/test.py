import gen_py.thrift.EngineServer
import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol


# Talk to a server via TCP sockets, using a binary protocol
transport = thrift.transport.TSocket.TSocket("localhost", 9090)
transport.open()
protocol = thrift.protocol.TBinaryProtocol.TBinaryProtocol(transport)

# Use the service we already defined
service = gen_py.thrift.EngineServer.Client(protocol)

# Retrieve something as well
players = service.getPlayers()

for player in players:
    print(player)
