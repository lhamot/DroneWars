from getClient import *

client = getClient('www.nogane.net', 9090)
# client = getClient('localhost', 9090)

# client.createUniverse(True) # Keep players
client.createUniverse(False)
