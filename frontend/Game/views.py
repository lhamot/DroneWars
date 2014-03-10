# -*- coding: utf-8 -*-

from datetime import timedelta, datetime

from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ugettext_lazy as N_

#from django.http import HttpResponse
#from django.shortcuts import render_to_response
from django.shortcuts import render, redirect
from django import forms
from django.contrib.sessions.models import Session
from django.utils import timezone
#from django.core.exceptions import PermissionDenied
import logging

import thrift.transport.TSocket
import thrift.protocol.TBinaryProtocol
import gen_py.thrift.EngineServer
from django.http import Http404
import DroneWars_frontend


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum
Event_Type = gen_py.thrift.ttypes.Event_Type
Player = gen_py.thrift.ttypes.Player
Sort_Type = gen_py.thrift.ttypes.Sort_Type

#trans = gettext.translation('DroneWars','I:/C/Bit_them_all/',["fr_FR.utf8"])
#trans.install()


def updateLastRequest(viewFunc):
    def wraped(request):
        request.session["last_request"] = datetime.utcnow()
        return viewFunc(request)
    return wraped


class SubscribeForm(forms.Form):
    login = forms.CharField(max_length=30, label=_("Login"))
    password = forms.CharField(max_length=30, label=_("Password"), widget=forms.PasswordInput)
    password2 = forms.CharField(max_length=30, label=_("Password2"), widget=forms.PasswordInput)
    #email = forms.EmailField()


class LoginForm(forms.Form):
    login = forms.CharField(max_length=30, label=_("Login"))
    password = forms.CharField(max_length=30, label=_("Password"), widget=forms.PasswordInput)

def createEngineClient():
    # Talk to a server via TCP sockets, using a binary protocol
    transport = thrift.transport.TSocket.TSocket(
        DroneWars_frontend.settings.SIMULATION['host'], 
        DroneWars_frontend.settings.SIMULATION['port'])
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
                    added = service.addPlayer(userInfo["login"].encode('utf-8'), 
                                              userInfo["password"].encode('utf-8'))
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
                optPlayer = service.logPlayer(userInfo["login"].encode('utf-8'), 
                                              userInfo["password"].encode('utf-8'))
                if optPlayer.player != None:
                    request.session.clear()
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


@updateLastRequest
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
        'player': player, 
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


def getEventAndFightReport(request, service, eventList):
    target_event = None
    fight_report = None
    if "event_id" in request.GET:
        event_id = int(request.GET["event_id"])
        for event in eventList:
            if event_id == int(event.id):
                target_event = event
        if target_event and target_event.type in {Event_Type.FleetLose, Event_Type.FleetWin, 
                                      Event_Type.PlanetLose, Event_Type.PlanetWin, 
                                      Event_Type.FightAvoided}:
            if target_event.value >= 0:
                fight_report = service.getFightReport(target_event.value);
    return target_event, fight_report
    

def prepareFightReport(fight_report, playerId):
    allyEnemyFilled = False
    def fillAllyEnemy(enemySet, fight_report):
        for index in enemySet:
            if index == -1:
                fight_report.planet.enemy = True
            else:
                fight_report.fleetList[index].enemy = True

    for fleetReport in fight_report.fleetList:
        if fleetReport.fightInfo.before.playerId == playerId:
            fillAllyEnemy(fleetReport.enemySet, fight_report);
            allyEnemyFilled = True
            break;
    if allyEnemyFilled == False and fight_report.planet and fight_report.planet.fightInfo.before.playerId == playerId:
        fillAllyEnemy(fight_report.planet.enemySet, fight_report);


@updateLastRequest
def PlanetView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    tab = request.GET["planet_coord"].split("_")
    tab = [int(val) for val in tab]
    targetCoord = gen_py.thrift.ttypes.Coord(tab[0], tab[1], tab[2])
        
    planetList = service.getPlanet(targetCoord)
    if len(planetList) == 0:
        raise Http404
    target = planetList[0]
    #if target.playerId != pid:
    #    raise PermissionDenied
        
    timeInfo = service.getTimeInfo();
    
    (target_event, fight_report) = getEventAndFightReport(request, service, target.eventList)
    
    if fight_report:
        prepareFightReport(fight_report, pid) 
    
    player = Player()
    player.id = pid; #Pour éviter une requete au serveur(Les raport de combat ont besoin de l'id)
    
    return render(request, 'planetview.html', {
        'player': player,
        'planet' : target,
        'timeInfo': timeInfo,
        'target_event': target_event,
        'fight_report': fight_report,
        'Event_Type': Event_Type,
        'cannonCount': sum(target.cannonTab),
        'shipCount': sum(target.hangar),
    })


def coordToTuple(coord):
    return coord.X, coord.Y, coord.Z


@updateLastRequest
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
        'player': player,
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
    

@updateLastRequest
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
    
    (target_event, fight_report) = getEventAndFightReport(request, service, fleet.eventList)
    
    if fight_report:
        prepareFightReport(fight_report, pid) 

    player = Player()
    player.id = pid; #Pour éviter une requete au serveur(Les raport de combat ont besoin de l'id)
    
    return render(request, 'fleetview.html', {
        'player': player,
        'fleet': fleet,
        'timeInfo': timeInfo,
        'planet': planet,
        'target_event': target_event,
        'fight_report': fight_report,
        'Event_Type': Event_Type,
    })    


CodeViewTutoTag = "CodeView"
@updateLastRequest
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
            'player': player,
            'level': plLvl,
            'timeInfo': timeInfo, 
            })
    else:
        return redirect("/ingame/codes/planets/blocks.html");


@updateLastRequest
def ReportsView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    eventList = service.getPlayerEvents(player.id)
    
    (target, fight_report) = getEventAndFightReport(request, service, eventList)
    
    if fight_report:
        prepareFightReport(fight_report, player.id)

    ReportViewTutoTag = "ReportView"
    if not ReportViewTutoTag in player.tutoDisplayed:
        helpMessage = _("REPORT_TUTOS")
        service.incrementTutoDisplayed(pid, ReportViewTutoTag)
    else:
        helpMessage = ""

    timeInfo = service.getTimeInfo()

    return render(request, 'reportsview.html', {
        'player': player,                                                    
        'eventList': eventList,
        'helpMessage': helpMessage,
        'target_event': target,
        'Event_Type': Event_Type,
        'fight_report': fight_report,
        'timeInfo': timeInfo,
    })


def checkIfTutoInfoSeen(service, player):
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0)
    tutoInfoTag = "TUTO_%i_INFO" % plLvl
    tutoInfoIsSeen = tutoInfoTag in player.tutoDisplayed
    if tutoInfoIsSeen == False:
        service.incrementTutoDisplayed(player.id, tutoInfoTag)
    return tutoInfoIsSeen


CoddingLevelTag = "BlocklyCodding"
@updateLastRequest
def BlocklyFleetsCodesView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()

    message = ""
    errorMessage = ""
    if request.method == "POST":
        if "save_button_error" in request.POST and len(request.POST["save_button_error"]):
            errorMessage = request.POST["save_button_error"]
            logger = logging.getLogger(__name__)
            logger.error("javascript error when saving blockly code : " + message);
        elif "blocklyXML" in request.POST:
            service.setPlayerFleetCode(pid, request.POST["scriptXML"].encode("utf8"))
            service.setPlayerFleetBlocklyCode(pid, request.POST["blocklyXML"].encode("utf8"))
            message = _("Code successfully saved")
   
   
    player = service.getPlayer(pid)
    codeData = player.fleetsCode
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0)
    tutosText = N_("BLOCKLY_TUTO_" + str(plLvl)) if plLvl <= 8 else None
    timeInfo = service.getTimeInfo()

    tutoInfoIsSeen = checkIfTutoInfoSeen(service, player)
     
    return render(request, 'codesview/blockly_fleet.html', {
        "name": "Fleet",
        "level": plLvl,
        "message": message,
        "errorMessage": errorMessage,
        "codeData": codeData,
        "tutosText": tutosText,
        "mode": "blockly",
        'timeInfo': timeInfo,
        'tutoInfoIsSeen': tutoInfoIsSeen,
        'player':player,
    })
    

@updateLastRequest
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
        'player':player,
    })


@updateLastRequest
def BlocklyPlanetsCodesView(request):
    FirstSaveTag = "firstSave"
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = None

    message = ""
    errorMessage = ""
    if request.method == "POST":
        if "save_button_error" in request.POST and len(request.POST["save_button_error"]):
            errorMessage = request.POST["save_button_error"]
            logger = logging.getLogger(__name__)
            logger.error("javascript error when saving blockly code : " + message);
        elif "blocklyXML" in request.POST:
            service.setPlayerPlanetCode(pid, request.POST["scriptXML"].encode("utf8"))
            service.setPlayerPlanetBlocklyCode(pid, request.POST["blocklyXML"].encode("utf8"))
            player = service.getPlayer(pid)
            message = _("Code successfully saved")
            firstSave = player.tutoDisplayed.get(FirstSaveTag, 0);
            if firstSave == 0:
                service.incrementTutoDisplayed(pid, FirstSaveTag)
                message = _("See in planets tab if the building is in progress")
   
    if player == None:
        player = service.getPlayer(pid)
    plLvl = player.tutoDisplayed.get(CoddingLevelTag, 0)
    codeData = player.planetsCode
    tutosText = N_("BLOCKLY_TUTO_" + str(plLvl)) if plLvl <= 8 else None
    
    helpMessage = _("CODE_TUTOS") if not CodeViewTutoTag in player.tutoDisplayed else None
    timeInfo = service.getTimeInfo()
    
    tutoInfoIsSeen = checkIfTutoInfoSeen(service, player)
    
    return render(request, 'codesview/blockly_planet.html', {
        "name": "Planet",
        "level": plLvl,
        "message": message,
        "errorMessage": errorMessage,
        "codeData": codeData,
        "tutosText": tutosText,
        "mode": "blockly",
        "helpMessage": helpMessage,
        "timeInfo": timeInfo,
        "tutoInfoIsSeen": tutoInfoIsSeen,
        'player':player,
    })


@updateLastRequest
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
        'player':player,
    })
    

@updateLastRequest
def ScoreView(request):
    service = createEngineClient()
    players = service.getPlayers()
    
    sort_order = request.GET["sort_order"] if "sort_order" in request.GET else "score"  
    asc = request.GET["asc"] != "True" if "asc" in request.GET else True
    key = {
        "login": lambda player: player.login,
        "score": lambda player: player.score,
        "exp": lambda player: player.experience,
    }
     
    players = sorted(players, key=key[sort_order], reverse=asc)
    timeInfo = service.getTimeInfo()
    
    sessions = Session.objects.filter(expire_date__gte=timezone.now())
    id_set = set()
    limit_time = datetime.utcnow() - timedelta(minutes=10)
    old_time = datetime.utcnow() - timedelta(days=1)
    for session in sessions:
        session_data = session.get_decoded()
        if session_data.get("last_request", old_time) > limit_time:
            playerID = session_data.get("PlayerID", None);
            if playerID != None:
                id_set.add(session_data["PlayerID"])
    
    for player in players:
        player.logged = player.id in id_set
        
    player = service.getPlayer(request.session["PlayerID"])
    
    return render(request, 'scoreview.html', {
        "player": player,
        "players": players,
        "asc": asc,
        "timeInfo": timeInfo,
    })


@updateLastRequest
def AccountView(request):
    service = createEngineClient()
    pid = request.session["PlayerID"]
    player = service.getPlayer(pid)
    timeInfo = service.getTimeInfo()
    
    message = ""
    if "erase_account" in request.POST:
        if service.eraseAccount(pid, request.POST["password"].encode('utf-8')) == False:
            message = _("Passwords don't match!")
        else:
            request.session.clear();
            return redirect("/");
    
    return render(request, 'account.html', {
        "player": player,
        "timeInfo": timeInfo,
        "message": message,
    })
    
XPPerSkillPoints = 1000

@updateLastRequest
def Skillsview(request):
    service = createEngineClient()
    pid = request.session["PlayerID"]
    player = service.getPlayer(pid)
    timeInfo = service.getTimeInfo()
    player.skillpoints /= XPPerSkillPoints
    
    if "buySkill" in request.GET:
        skillID = int(request.GET["buySkill"])
        if service.buySkill(pid, skillID):
            player = service.getPlayer(pid)
            player.skillpoints /= XPPerSkillPoints

    return render(request, 'skillsview.html', {
        "player": player,
        "timeInfo": timeInfo,
        "skill_list": player.skilltab,
        "prev_skill_point" : (player.experience / XPPerSkillPoints) * XPPerSkillPoints,
        "next_skill_point" : (player.experience / XPPerSkillPoints + 1) * XPPerSkillPoints,
        "skill_point_progress" : ((player.experience * 100) / XPPerSkillPoints) % 100,
    })
    
    