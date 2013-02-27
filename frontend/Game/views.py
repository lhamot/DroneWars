# -*- coding: utf-8 -*-

from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ugettext_lazy as N_

#from django.http import HttpResponse
#from django.shortcuts import render_to_response
from django.shortcuts import render, redirect
from django import forms
from django.contrib.sessions.models import Session
from django.utils import timezone
import logging

import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum
Event_Type = gen_py.thrift.ttypes.Event_Type
Player = gen_py.thrift.ttypes.Player
Sort_Type = gen_py.thrift.ttypes.Sort_Type

#trans = gettext.translation('DroneWars','I:/C/Bit_them_all/',["fr_FR.utf8"])
#trans.install()


class SubscribeForm(forms.Form):
    login = forms.CharField(max_length=100, label=_("Login"))
    password = forms.CharField(label=_("Password"), widget=forms.PasswordInput)
    password2 = forms.CharField(label=_("Password2"), widget=forms.PasswordInput)
    #email = forms.EmailField()


class LoginForm(forms.Form):
    login = forms.CharField(max_length=100, label=_("Login"))
    password = forms.CharField(label=_("Password"), widget=forms.PasswordInput)

def createEngineClient():
    # Talk to a server via TCP sockets, using a binary protocol
    transport = thrift.transport.TSocket.TSocket("localhost", 9090)
    transport.open()
    protocol = thrift.protocol.TBinaryProtocol.TBinaryProtocol(transport)

    # Use the service we already defined
    service = gen_py.thrift.EngineServer.Client(protocol)
    return service


def OutPage(request):
    subForm = SubscribeForm()
    logForm = LoginForm()
    regMessage = ""
    regMessageState = False
    logMessage = ""
    if request.method == 'POST':
        if "subscribe_action" in request.POST:
            subForm = SubscribeForm(request.POST)
            if subForm.is_valid():
                userInfo = subForm.cleaned_data;
                if userInfo["password"] != userInfo["password2"]:
                    regMessage = _("Passwords don't match!")
                else:
                    service = createEngineClient()
                    added = service.addPlayer(userInfo["login"], userInfo["password"])
                    if added:
                        regMessage = _("Registration successful")
                        regMessageState = True
                    else:
                        regMessage = _("Login still exist.")
        elif "login_action" in request.POST:
            logForm = LoginForm(request.POST)
            if logForm.is_valid():
                userInfo = logForm.cleaned_data;
                service = createEngineClient()
                optPlayer = service.logPlayer(userInfo["login"], userInfo["password"])
                if optPlayer.player != None:
                    request.session["PlayerID"] = optPlayer.player.id
                    return redirect("/ingame/codes.html") 
                else:
                    logMessage = _("Wrong login or password.")
        else:
            raise RuntimeError("Unexcpected request")           

      
    return render(request, 'index.html', {
        'subForm': subForm,
        'logForm': logForm,
        'regMessage': regMessage,
        'logMessage': logMessage,
        'regMessageState': regMessageState
    })


def getSortInfo(request, default_sort):
    page = 0
    if "page" in request.GET:
        page = int(request.GET["page"])
    sort = default_sort
    if "sort" in request.GET:
        sort = int(request.GET["sort"])
    asc = True
    if "asc" in request.GET:
        asc = request.GET["asc"] == "True"
    return page, sort, asc


def getPageList(current_page, page_count):
    page_list = [current_page]
    index = current_page - 1
    step = 1
    while index >= 0:
        page_list.insert(0, index)
        index -= step
        step += 1
    index = current_page + 1
    step = 1
    while index < page_count:
        page_list.append(index)
        index += step
        step += 1
    return page_list


def PlanetListView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    
    page, sort, asc = getSortInfo(request, Sort_Type.Name)

    playerPlanets = service.getPlayerPlanets(pid, page * 10, (page + 1) * 10, sort, asc)
    
    PlanetViewTutoTag = "PlanetView"
    if not PlanetViewTutoTag in player.tutoDisplayed:
        helpMessage = _("PLANET_TUTOS")
        service.incrementTutoDisplayed(pid, PlanetViewTutoTag);
    else:
        helpMessage = ""
        
    timeInfo = service.getTimeInfo();
    
    pagecount = (playerPlanets.planetCount + 9) / 10

    pageList = getPageList(page, pagecount)
    
    titleAfter = [""] * 7
    titleAfter[sort] = '<i class="icon-chevron-up"></i>' if asc == True else '<i class="icon-chevron-down"></i>'
    titleOrder = [asc] * 7
    titleOrder[sort] = titleOrder[sort] == False 
    
    return render(request, 'planetsview.html', {
        'planetList': playerPlanets.planetList,
        'helpMessage': helpMessage,
        'timeInfo': timeInfo,
        'prevpage': page - 1,
        'currentpage': page,
        'nextpage': page + 1,
        'pagecount': pagecount,
        'pageList': pageList,
        'sort': sort,
        'asc': asc,
        'titleAfter': titleAfter,
        'titleOrder': titleOrder,
        'helpMessage': helpMessage,
    })


def PlanetView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    tab = request.GET["planet_coord"].split("_")
    tab = [int(val) for val in tab]
    targetCoord = gen_py.thrift.ttypes.Coord(tab[0], tab[1], tab[2])
        
    target = service.getPlanet(targetCoord)[0]
    if target.playerId != pid:
        raise RuntimeError("Try to access to a not owned planet")
        
    timeInfo = service.getTimeInfo();
    
    return render(request, 'planetview.html', {
        'planet' : target,
        'timeInfo': timeInfo,
    })


def coordToTuple(coord):
    return coord.X, coord.Y, coord.Z


def FleetListView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    
    page, sort, asc = getSortInfo(request, Sort_Type.X)

    playerFleets = service.getPlayerFleets(pid, page * 10, (page + 1) * 10, sort, asc)
    
    planetMap = {}
    for planet in playerFleets.planetList:
        planetMap[coordToTuple(planet.coord)] = planet
    planetFleetPairs = []
    for fleet in playerFleets.fleetList:
        if coordToTuple(fleet.coord) in planetMap:
            planetFleetPairs.append((fleet, planetMap[coordToTuple(fleet.coord)]))
        else:
            planetFleetPairs.append((fleet, None))
    
    player = service.getPlayer(pid)
    FleetViewTutoTag = "FleetView"
    if not FleetViewTutoTag in player.tutoDisplayed:
        helpMessage = _("FLEET_TUTOS")
        service.incrementTutoDisplayed(pid, FleetViewTutoTag);
    else:
        helpMessage = ""
        
    timeInfo = service.getTimeInfo();
    
    pagecount = (playerFleets.fleetCount + 9) / 10

    pageList = getPageList(page, pagecount)
    
    titleAfter = [""] * 7
    titleAfter[sort] = '<i class="icon-chevron-up"></i>' if asc == True else '<i class="icon-chevron-down"></i>'
    titleOrder = [asc] * 7
    titleOrder[sort] = titleOrder[sort] == False
        
    return render(request, 'fleetsview.html', {
        'planetFleetPairs': planetFleetPairs,
        'timeInfo': timeInfo,
        'prevpage': page - 1,
        'currentpage': page,
        'nextpage': page + 1,
        'pagecount': pagecount,
        'pageList': pageList,
        'sort': sort,
        'asc': asc,
        'titleAfter': titleAfter,
        'titleOrder': titleOrder,
        'helpMessage': helpMessage,
    })
    

def FleetView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    fleetID = int(request.GET["fleet"])
    fleet = service.getFleet(fleetID)
    if fleet.playerId != pid:
        raise RuntimeError("Try to access to a not owned fleet")
    planet = service.getPlanet(fleet.coord)
    planet = None if len(planet) == 0 else planet[0]
        
    timeInfo = service.getTimeInfo();
    
    return render(request, 'fleetview.html', {
        'fleet': fleet,
        'timeInfo': timeInfo,
        'planet': planet,
    })    


CodeViewTutoTag = "CodeView"
def CodesView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    if not CodeViewTutoTag in player.tutoDisplayed:
        service.incrementTutoDisplayed(pid, CodeViewTutoTag);
        
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
    
    #Ce test permet d'afficher immediatement la page BlocklyPlanets pour les debutant
    if plLvl >= 3:
        timeInfo = service.getTimeInfo();
        return render(request, 'codesview.html', {
            'level': plLvl,
            'timeInfo': timeInfo, 
            })
    else:
        return redirect("/ingame/codes/planets/blocks.html");


def ReportsView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    
    target = None
    fight_report = None
    if "event_id" in request.GET:
        event_id = int(request.GET["event_id"])
        for event in player.eventList:
            if event_id == int(event.id):
                target = event
        if target and target.type in {Event_Type.FleetLose, Event_Type.FleetWin, 
                                      Event_Type.PlanetLose, Event_Type.PlanetWin}:
            fight_report = service.getFightReport(target.value);

    ReportViewTutoTag = "ReportView"
    if not ReportViewTutoTag in player.tutoDisplayed:
        helpMessage = _("REPORT_TUTOS")
        service.incrementTutoDisplayed(pid, ReportViewTutoTag)
    else:
        helpMessage = ""

    timeInfo = service.getTimeInfo()

    return render(request, 'reportsview.html', {
        'player': player,
        'helpMessage': helpMessage,
        'target': target,
        'Event_Type': Event_Type,
        'fight_report': fight_report,
        'timeInfo': timeInfo,
    })


CoddingLevelTag = "BlocklyCodding"
def BlocklyFleetsCodesView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    message = ""
    if request.method == "POST":
        if "blocklyXML" in request.POST:
            service.setPlayerFleetBlocklyCode(pid, request.POST["blocklyXML"].encode("utf8"))
            service.setPlayerFleetCode(pid, request.POST["scriptXML"].encode("utf8"))
            message = _("Code successfully saved")
   
   
    player = service.getPlayer(pid)
    codeData = player.fleetsCode
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
    tutosText = N_("BLOCKLY_TUTO_" + str(plLvl)) if plLvl <= 8 else None
    timeInfo = service.getTimeInfo()
     
    return render(request, 'codesview/blockly_fleet.html', {
        "name": "Fleet",
        "level": plLvl,
        "message": message,
        "codeData": codeData,
        "tutosText": tutosText,
        "mode": "blockly",
        'timeInfo': timeInfo,
    })
    

def TextFleetsCodesView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    message = ""
    if request.method == "POST":
        if "save_button" in request.POST:
            service.setPlayerFleetCode(pid, request.POST["TextArea"].encode("utf8"))
            message = _("Code successfully saved")

    player = service.getPlayer(pid)
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
    codeData = player.fleetsCode
    timeInfo = service.getTimeInfo()

    return render(request, 'codesview/text.html', {
        "name": "Fleet",
        "codeData": codeData,
        "message": message,
        "script": codeData.code,
        "level": plLvl,
        "mode": "text",
        "timeInfo": timeInfo,
    })


def BlocklyPlanetsCodesView(request):
    FirstSaveTag = "firstSave"
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)

    message = ""
    if request.method == "POST":
        if "save_button_error" in request.POST and len(request.POST["save_button_error"]):
            message = request.POST["save_button_error"]
            logger = logging.getLogger(__name__)
            logger.error("javascript error when saving blockly code : " + message);
        elif "blocklyXML" in request.POST:
            service.setPlayerPlanetBlocklyCode(pid, request.POST["blocklyXML"].encode("utf8"))
            service.setPlayerPlanetCode(pid, request.POST["scriptXML"].encode("utf8"))
            message = _("Code successfully saved")
            firstSave = player.tutoDisplayed.get(FirstSaveTag, 0);
            if firstSave == 0:
                service.incrementTutoDisplayed(pid, FirstSaveTag)
                message = _("See in planets tab if the building is in progress")
   
    player = service.getPlayer(pid) #Redemendé car code modifié
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0)
    codeData = player.planetsCode
    tutosText = N_("BLOCKLY_TUTO_" + str(plLvl)) if plLvl <= 8 else None
    
    helpMessage = _("CODE_TUTOS") if not CodeViewTutoTag in player.tutoDisplayed else None
    timeInfo = service.getTimeInfo()
            
    return render(request, 'codesview/blockly_planet.html', {
        "name": "Planet",
        "level": plLvl,
        "message": message,
        "codeData": codeData,
        "tutosText": tutosText,
        "mode": "blockly",
        "helpMessage": helpMessage,
        "timeInfo": timeInfo,
    })


def TextPlanetsCodesView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    
    message = ""
    if request.method == "POST":
        if "save_button" in request.POST:
            service.setPlayerPlanetCode(pid, request.POST["TextArea"].encode("utf8"))
            message = _("Code successfully saved")
    
    
    player = service.getPlayer(pid)
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
    codeData = player.planetsCode
    timeInfo = service.getTimeInfo()

    return render(request, 'codesview/text.html', {
        "name": "Planet",
        "codeData": codeData,
        "message": message,
        "script": codeData.code,
        "level": plLvl,    
        "mode": "text",
        "timeInfo": timeInfo,         
    })
    

def ScoreView(request):
    #pid = request.session["PlayerID"]
    service = createEngineClient()
    players = service.getPlayers()
    
    sort_order = request.GET["sort_order"] if "sort_order" in request.GET else "score"  
    asc = request.GET["asc"] != "True" if "asc" in request.GET else True
    key = {
        "login": lambda player: player.login,
        "score": lambda player: player.score,
    }
     
    players = sorted(players, key=key[sort_order], reverse=asc)
    timeInfo = service.getTimeInfo()
    
    sessions = Session.objects.filter(expire_date__gte=timezone.now())
    id_set = set()
    for session in sessions:
        id_set.add(session.get_decoded()["PlayerID"])
    
    for player in players:
        player.logged = player.id in id_set 
    
    return render(request, 'scoreview.html', {
        "players": players,
        "asc": asc,
        "timeInfo": timeInfo,
    })
    
    