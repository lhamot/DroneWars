#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
import os
import sys

if __name__ == "__main__":
    os.environ.setdefault("DJANGO_SETTINGS_MODULE", "DroneWars_frontend.settings")

    from django.core.management import execute_from_command_line

    execute_from_command_line(sys.argv)
