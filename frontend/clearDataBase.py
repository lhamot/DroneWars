#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from getClient import *

client = getClient('www.nogane.net', 9090)
# client = getClient('localhost', 9090)

# client.createUniverse(True) # Keep players
client.createUniverse(False)
