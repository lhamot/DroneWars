from django.conf.urls import patterns, include, url
import gettext
import os

# Uncomment the next two lines to enable the admin:
# from django.contrib import admin
# admin.autodiscover()


urlpatterns = patterns('',
    # Examples:
    url(r'^$', 'Game.views.OutPage'),
    
    url(r'^i18n/', include('django.conf.urls.i18n')),
    
    url(r'^ingame/codes/fleets/blocks.html',       'Game.views.BlocklyFleetsCodesView'),
    url(r'^ingame/codes/fleets/text.html',         'Game.views.TextFleetsCodesView'),
    url(r'^ingame/codes/planets/blocks.html',      'Game.views.BlocklyPlanetsCodesView'),
    url(r'^ingame/codes/planets/text.html',        'Game.views.TextPlanetsCodesView'),

    
    url(r'^ingame/planets.html', 'Game.views.PlanetsView'),
    url(r'^ingame/fleets.html', 'Game.views.FleetsView'),
    url(r'^ingame/codes.html', 'Game.views.CodesView'),
    url(r'^ingame/reports.html', 'Game.views.ReportsView'),

    # url(r'^DroneWars_frontend/', include('DroneWars_frontend.foo.urls')),

    # Uncomment the admin/doc line below to enable admin documentation:
    # url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
    # url(r'^admin/', include(admin.site.urls)),
)
