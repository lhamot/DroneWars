#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
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