# -*- coding: utf-8 -*-

from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ugettext_lazy as N_

#from django.http import HttpResponse
#from django.shortcuts import render_to_response
from django.shortcuts import render, redirect
from django import forms

import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum
Event_Type = gen_py.thrift.ttypes.Event_Type
Player = gen_py.thrift.ttypes.Player

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


PlanetImageCount = 23
def PlanetsView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        planetList = service.getPlayerPlanets(pid)
        player = service.getPlayer(pid)
        target = None
        targetCoord = None
        if "planet_coord" in request.GET:
            tab = request.GET["planet_coord"].split("_")
            tab = [int(val) for val in tab]
            targetCoord = gen_py.thrift.ttypes.Coord(tab[0], tab[1], tab[2])
        else:
            targetCoord = player.mainPlanet
        for planet in planetList:
            planetHash = planet.coord.X + (planet.coord.Y * 1000) + (planet.coord.Z * 1000000) 
            planet.imgNumber = planetHash % PlanetImageCount;
            if targetCoord and targetCoord == planet.coord:
                target = planet

        PlanetViewTutoTag = "PlanetView"
        if not PlanetViewTutoTag in player.tutoDisplayed:
            helpMessage = _("PLANET_TUTOS")
            service.incrementTutoDisplayed(pid, PlanetViewTutoTag);
        else:
            helpMessage = ""
            
        timeInfo = service.getTimeInfo();
        
        return render(request, 'planetsview.html', {
            'planetList': planetList,
            'planet' : target,
            'helpMessage': helpMessage,
            'timeInfo': timeInfo
    })


def FleetsView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        fleetList = service.getPlayerFleets(pid)

        if "fleet" in request.GET:
            fleetID = int(request.GET["fleet"])
            fleet = next((f for f in fleetList if f.id == fleetID), None)
        else:
            fleet = None

        player = service.getPlayer(pid)
        FleetViewTutoTag = "FleetView"
        if not FleetViewTutoTag in player.tutoDisplayed:
            helpMessage = _("FLEET_TUTOS")
            service.incrementTutoDisplayed(pid, FleetViewTutoTag);
        else:
            helpMessage = ""
        
        return render(request, 'fleetsview.html', {
            'fleetList': fleetList,
            'fleet': fleet,
            'helpMessage': helpMessage
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
            return render(request, 'codesview.html', {
                'level': plLvl 
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
            service.incrementTutoDisplayed(pid, ReportViewTutoTag);
        else:
            helpMessage = ""

        return render(request, 'reportsview.html', {
            'player': player,
            'helpMessage': helpMessage,
            'target': target,
            'Event_Type': Event_Type,
            'fight_report': fight_report
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
         
        return render(request, 'codesview/blockly_fleet.html', {
            "name": "Fleet",
            "level": plLvl,
            "message": message,
            "codeData": codeData,
            "tutosText": tutosText,
            "mode": "blockly"
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

        return render(request, 'codesview/text.html', {
            "name": "Fleet",
            "codeData": codeData,
            "message": message,
            "script": codeData.code,
            "level": plLvl,
            "mode": "text",
            
    })


def BlocklyPlanetsCodesView(request):
        FirstSaveTag = "firstSave"
        pid = request.session["PlayerID"]
        service = createEngineClient()
        player = service.getPlayer(pid)
    
        message = ""
        if request.method == "POST":
            if "blocklyXML" in request.POST:
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
                
        return render(request, 'codesview/blockly_planet.html', {
            "name": "Planet",
            "level": plLvl,
            "message": message,
            "codeData": codeData,
            "tutosText": tutosText,
            "mode": "blockly",
            "helpMessage": helpMessage,
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
    
        return render(request, 'codesview/text.html', {
            "name": "Planet",
            "codeData": codeData,
            "message": message,
            "script": codeData.code,
            "level": plLvl,    
            "mode": "text",        
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
    
    return render(request, 'scoreview.html', {
        "players": players,
        "asc": asc
    })
    
    