from django.utils.translation import ugettext_lazy as _
import django.utils.translation

#from django.http import HttpResponse
#from django.shortcuts import render_to_response
from django.shortcuts import render, redirect
from django import forms

import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer
from gen_py.thrift.ttypes import *


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum

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
                    return redirect("/ingame/planets.html") 
                else:
                    logMessage = _("Wrong login or password.")
        else:
            raise RuntimeError("Unexcpected request")           

      
    return render(request, 'index.html', {
        'subForm': subForm,
        'logForm': logForm,
        'regMessage': regMessage,
        'logMessage': logMessage
    })


PlanetImageCount = 23
def PlanetsView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        planetList = service.getPlayerPlanets(pid)
        target = None
        targetCoord = None
        if "planet_coord" in request.GET:
            tab = request.GET["planet_coord"].split("_")
            tab = [int(val) for val in tab]
            targetCoord = gen_py.thrift.ttypes.Coord(tab[0], tab[1], tab[2])
        print len(planetList)
        for planet in planetList:
            planetHash = planet.coord.X + (planet.coord.Y * 1000) + (planet.coord.Z * 1000000) 
            planet.imgNumber = planetHash % PlanetImageCount;
            if targetCoord and targetCoord == planet.coord:
                target = planet
                for task in planet.taskQueue:
                    task.name = PlanetTask_Enum._VALUES_TO_NAMES[task.type]
                    if task.type == PlanetTask_Enum.UpgradeBuilding:
                        task.what = Building_Enum._VALUES_TO_NAMES[task.value]
                    elif task.type == PlanetTask_Enum.MakeShip:
                        task.what = Ship_Enum._VALUES_TO_NAMES[task.value]
                    elif task.type == PlanetTask_Enum.MakeCannon:
                        task.what = Cannon_Enum._VALUES_TO_NAMES[task.value]
                    else:
                        raise AssertionError("Unconsistent PlanetTask")
        
        return render(request, 'planetsview.html', {
            'planetList': planetList,
            'planet' : target,
    })


def FleetsView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        fleetList = service.getPlayerFleets(pid)

        if "fleet" in request.GET:
            fleetID = int(request.GET["fleet"])
            print fleetID
            fleet = next((f for f in fleetList if f.id == fleetID), None)
        else:
            fleet = None
        
        return render(request, 'fleetsview.html', {
            'fleetList': fleetList,
            'fleet': fleet
    })


def CodesView(request):
        return render(request, 'codesview.html', {
    })


def ReportsView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        player = service.getPlayer(pid)
        
        return render(request, 'reportsview.html', {
            'player': player
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
       
        codeData = service.getPlayerFleetCode(pid)
        code = codeData.blocklyCode.replace('\n', '').replace('\r', '')
        
        player = service.getPlayer(pid)
        plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
        return render(request, 'codesview/blockly.html', {
            "name": "Fleet",
            "level": plLvl,
            "blockly_code" : code,
            "message": message
    })
    

def TextFleetsCodesView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
    
        message = ""
        if request.method == "POST":
            if "save_button" in request.POST:
                service.setPlayerFleetCode(pid, request.POST["TextArea"].encode("utf8"))
                message = _("Code successfully saved")

        codeData = service.getPlayerFleetCode(pid)

        return render(request, 'codesview/text.html', {
            "name": "Fleet",
            "codeData": codeData,
            "message": message,
            "script": codeData.code
    })


def BlocklyPlanetsCodesView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
    
        message = ""
        if request.method == "POST":
            if "blocklyXML" in request.POST:
                service.setPlayerPlanetBlocklyCode(pid, request.POST["blocklyXML"].encode("utf8"))
                service.setPlayerPlanetCode(pid, request.POST["scriptXML"].encode("utf8"))
                message = _("Code successfully saved")
       
        codeData = service.getPlayerPlanetCode(pid)
        code = codeData.blocklyCode.replace('\n', '').replace('\r', '')
        
        player = service.getPlayer(pid)
        plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0);
                
        return render(request, 'codesview/blockly.html', {
            "name": "Planet",
            "level": plLvl,
            "blockly_code" : code,
            "message": message
    })


def TextPlanetsCodesView(request):
        pid = request.session["PlayerID"]
        service = createEngineClient()
        
        message = ""
        if request.method == "POST":
            if "save_button" in request.POST:
                service.setPlayerPlanetCode(pid, request.POST["TextArea"].encode("utf8"))
                message = _("Code successfully saved")
        
        
        codeData = service.getPlayerPlanetCode(pid)
    
        return render(request, 'codesview/text.html', {
            "name": "Planet",
            "codeData": codeData,
            "message": message,
            "script": codeData.code
        })
    
    
    