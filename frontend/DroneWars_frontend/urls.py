#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
from django.conf.urls import include, url
import Game.views
import Game.info_views
import Game.community_views
#from django.views.generic import ListView, CreateView, UpdateView, DeleteView
#import gettext
#import os
#from Game.models import Ticket

# Uncomment the next two lines to enable the admin:
# from django.contrib import admin
# admin.autodiscover()


urlpatterns = [
    # Examples:
    url(r'^$', Game.views.OutPage),
    
    url(r'^i18n/', include('django.conf.urls.i18n')),
    
    url(r'^ingame/codes/fleets/blocks.html',       Game.views.BlocklyFleetsCodesView),
    url(r'^ingame/codes/fleets/text.html',         Game.views.TextFleetsCodesView),
    url(r'^ingame/codes/planets/blocks.html',      Game.views.BlocklyPlanetsCodesView),
    url(r'^ingame/codes/planets/text.html',        Game.views.TextPlanetsCodesView),

    
    url(r'^ingame/planetlist.html', Game.views.PlanetListView),
    url(r'^ingame/planet.html',     Game.views.PlanetView),
    url(r'^ingame/fleetlist.html',  Game.views.FleetListView),
    url(r'^ingame/fleet.html',      Game.views.FleetView),
    url(r'^ingame/codes.html',      Game.views.CodesView),
    url(r'^ingame/reports.html',    Game.views.ReportsView),
    url(r'^ingame/score.html',      Game.views.ScoreView),
    url(r'^ingame/account.html',    Game.views.AccountView),
    url(r'^ingame/buildings.html',  Game.info_views.BuildingsView),
    url(r'^ingame/cannons.html',    Game.info_views.CannonsView),
    url(r'^ingame/ships.html',      Game.info_views.ShipsView),
    url(r'^ingame/skilllist.html',  Game.views.Skillsview),
    
    url(r'^ingame/messages.html',       Game.community_views.MessagesView),
    url(r'^ingame/create_message.html', Game.community_views.CreateMessageView),
    
    url(r'^ingame/friends.html', Game.community_views.FriendsView),
    
    url(r'^ingame/alliance.html', Game.community_views.AllianceView),
    
    #url(r'^ingame/ticket_list.html', ListView.as_view(model=Ticket)),
    #url(r'^ingame/add_ticket.html', CreateView.as_view(model=Ticket, success_url="/ingame/ticket_list.html")),
    #url(r'^ingame/view_ticket.html/(?P<pk>\d+)', UpdateView.as_view(model=Ticket, success_url="/ingame/ticket_list.html")),
    #url(r'^ingame/del_ticket.html/(?P<pk>\d+)', DeleteView.as_view(model=Ticket, success_url="/ingame/ticket_list.html")),


    # url(r'^DroneWars_frontend/', include('DroneWars_frontend.foo.urls')),

    # Uncomment the admin/doc line below to enable admin documentation:
    # url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
    # url(r'^admin/', include(admin.site.urls)),
]
