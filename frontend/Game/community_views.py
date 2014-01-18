# -*- coding: utf-8 -*-

from django.utils.translation import ugettext_lazy as _

from django.shortcuts import render, redirect
from django import forms

import gen_py.thrift.EngineServer


Building_Enum = gen_py.thrift.ttypes.Building_Enum
Cannon_Enum = gen_py.thrift.ttypes.Cannon_Enum
PlanetTask_Enum = gen_py.thrift.ttypes.PlanetTask_Enum
Ship_Enum = gen_py.thrift.ttypes.Ship_Enum
Event_Type = gen_py.thrift.ttypes.Event_Type
Player = gen_py.thrift.ttypes.Player
Sort_Type = gen_py.thrift.ttypes.Sort_Type


from views import updateLastRequest, createEngineClient


@updateLastRequest
def MessagesView(request):
    service = createEngineClient()
    pid = request.session["PlayerID"]
    messages = service.getMessages(pid)
    return render(request, 'message_list.html', {
        'messages': messages,
    })


class MessageForm(forms.Form):
    subject = forms.CharField(max_length=80, label=_("Subject"))
    message = forms.CharField(label=_("Message"), widget=forms.Textarea)


@updateLastRequest
def CreateMessageView(request):
    messForm = MessageForm()
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid) 
    alliance = service.getAlliance(player.allianceID)
    alert = None
    
    if request.method == 'GET':
        recipientID = int(request.GET["recipientID"]) 
        recipientLogin = service.getPlayer(recipientID).login
    elif request.method == 'POST':
        recipientID = int(request.POST["recipientID"]) 
        recipientLogin = request.POST["recipientLogin"]
        if "send_message" in request.POST: 
            messForm = MessageForm(request.POST)
            if messForm.is_valid():
                messInfo = messForm.cleaned_data
                service.addMessage(pid, 
                                   recipientID, 
                                   messInfo["subject"].encode('utf-8'), 
                                   messInfo["message"].encode('utf-8'))
                messages = service.getMessages(pid)
                return render(request, 'message_list.html', {
                    'messages': messages,
                    'alert': _("Message successfully sent"),
                })
        elif "send_friendship_request" in request.POST:
            playerA = int(request.POST["senderID"])
            if playerA == pid:
                playerB = recipientID
                service.addFriendshipRequest(playerA, playerB)
                alert = _("Request sent")
        elif "transfert_alliance" in request.POST:
            service.transfertAlliance(alliance.id, recipientID)
            alliance = service.getAlliance(player.allianceID)

    inspectedPlayer = service.getPlayer(recipientID)
    return render(request, 'create_message.html', {
        'player': player,
        'inspectedPlayer': inspectedPlayer,
        'alliance': alliance,
        'senderID': pid,
        'recipientID': recipientID, 
        'recipientLogin': recipientLogin, 
        'messForm': messForm,
        'alert': alert,
    })


@updateLastRequest
def FriendsView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    alert = None
    
    if "accept" in request.GET:
        accept = request.GET["accept"] == "True"
        sender = int(request.GET["sender"])
        service.acceptFriendshipRequest(sender, pid, accept)
    elif "cancel" in request.GET:
        recipient = int(request.GET["recipientID"])
        service.acceptFriendshipRequest(pid, recipient, False)
    elif "close" in request.GET:
        friend = int(request.GET["friend"])
        service.closeFriendship(friend, pid)
    
    requests = service.getFriendshipRequest(pid)
    frienships = service.getFriends(pid)
    return render(request, 'friendships.html', {
        'requests': requests, 
        'frienships': frienships,
        'alert': alert, 
    })


class AllianceForm(forms.Form):
    name = forms.CharField(max_length=30, label=_("Name"))
    description = forms.CharField(label=_("Description"), widget=forms.Textarea)


@updateLastRequest
def AllianceView(request):
    pid = request.session["PlayerID"]
    service = createEngineClient()
    player = service.getPlayer(pid)
    allianceID =  player.allianceID
    form = None
    alert = None
    
    if "alliance" in request.GET:
        allianceID = int(request.GET["alliance"])
    elif "quit" in request.GET:
        service.quitAlliance(pid)
        allianceID = 0
    elif "join" in request.GET:
        allianceID = int(request.GET["join"])
        service.joinAlliance(pid, allianceID)
        player.allianceID = allianceID
    elif request.method == "POST":
        form = AllianceForm(request.POST)
        if form.is_valid():
            alliInfo = form.cleaned_data
            if "create" in request.POST:
                allianceID = service.addAlliance(pid, 
                                                 alliInfo["name"].encode('utf-8'), 
                                                 alliInfo["description"].encode('utf-8'))
                if allianceID == 0:
                    alert = _("Alliance name still exists or you still own an alliance")
                else:
                    return redirect("alliance.html")
            elif "update" in request.POST:
                alliance = gen_py.thrift.ttypes.Alliance()
                if allianceID == int(request.POST["allianceID"]):
                    alliance.id = allianceID
                    alliance.name = alliInfo["name"].encode('utf-8')
                    alliance.description = alliInfo["description"].encode('utf-8')
                    service.updateAlliance(alliance)
                
        
    alliance = service.getAlliance(allianceID)
    if not form:
        form = AllianceForm({"name": alliance.name, 
                             "description": alliance.description,
                             "masterLogin": alliance.masterLogin
                             })
    return render(request, 'alliance.html', {
        'player': player, 
        'alliance': alliance,
        'form': form,
        'alert': alert,
    })

