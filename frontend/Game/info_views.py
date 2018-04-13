#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from datetime import timedelta, datetime

from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ugettext_lazy as N_

#from django.http import HttpResponse
#from django.shortcuts import render_to_response
from django.shortcuts import render, redirect
from django import forms
from django.contrib.sessions.models import Session
from django.utils import timezone
import logging
import math

import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer

from views import updateLastRequest, createEngineClient


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum
Event_Type = gen_py.thrift.ttypes.Event_Type
Player = gen_py.thrift.ttypes.Player
Sort_Type = gen_py.thrift.ttypes.Sort_Type
RessourceSet = gen_py.thrift.ttypes.RessourceSet


def getBuilingPrice(building, targetLevel):
    coef = math.pow(building.coef, targetLevel - 1.)
    result = RessourceSet()
    result.tab = [int(val * coef) for val in building.price.tab]
    return result


@updateLastRequest
def BuildingsView(request):
    service = createEngineClient()
    buildings = service.getBuildingsInfo()
    pid = request.session["PlayerID"]
    player = service.getPlayer(pid)
    
    #buildings = [b for b in buildings if b.index in {0, 1, 4}]
    prices = None
    target = None
    if "bid" in request.GET:
        bid = int(request.GET["bid"])
        target = buildings[bid]
        prices = [getBuilingPrice(target, i) for i in range(1, 60)]

    return render(request, 'buildingsview.html', {
        'player': player,
        'buildings': buildings,
        'prices': prices,
        'target': target
    })


@updateLastRequest
def CannonsView(request):
    service = createEngineClient()
    cannons = service.getCannonsInfo()
    pid = request.session["PlayerID"]
    player = service.getPlayer(pid)
    
    #cannons = [b for b in cannons if b.index in {0, 1, 4}]

    return render(request, 'cannonsview.html', {
        'player': player,
        'cannons': cannons
    })    
   

@updateLastRequest
def ShipsView(request):
    service = createEngineClient()
    ships = service.getShipsInfo()
    pid = request.session["PlayerID"]
    player = service.getPlayer(pid)
    
    #cannons = [b for b in cannons if b.index in {0, 1, 4}]

    return render(request, 'shipsview.html', {
        'player': player,
        'ships': ships
    })
    
"""    
@updateLastRequest
def CannonsView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    
    (target, fight_report) = getEventAndFightReport(request, service, player) 

    ReportViewTutoTag = "ReportView"
    if not ReportViewTutoTag in player.tutoDisplayed:
        helpMessage = _("REPORT_TUTOS")
        service.incrementTutoDisplayed(pid, ReportViewTutoTag, 1)
    else:
        helpMessage = ""

    timeInfo = service.getTimeInfo()

    return render(request, 'reportsview.html', {
        'player': player,
        'helpMessage': helpMessage,
        'target_event': target,
        'Event_Type': Event_Type,
        'fight_report': fight_report,
        'timeInfo': timeInfo,
    })
    
    
    
@updateLastRequest
def ShipsView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    
    (target, fight_report) = getEventAndFightReport(request, service, player) 

    ReportViewTutoTag = "ReportView"
    if not ReportViewTutoTag in player.tutoDisplayed:
        helpMessage = _("REPORT_TUTOS")
        service.incrementTutoDisplayed(pid, ReportViewTutoTag, 1)
    else:
        helpMessage = ""

    timeInfo = service.getTimeInfo()

    return render(request, 'reportsview.html', {
        'player': player,
        'helpMessage': helpMessage,
        'target_event': target,
        'Event_Type': Event_Type,
        'fight_report': fight_report,
        'timeInfo': timeInfo,
    })
"""
