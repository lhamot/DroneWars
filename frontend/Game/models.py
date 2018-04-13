#!/usr/bin/env python
#-*- coding: utf-8 -*-
#
# Copyright (c) 2018 Loïc HAMOT
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

from django.db import models
from django.utils.translation import ugettext_lazy as _

class Ticket(models.Model):
    SEVERITY_CHOICES = (
        # Translators: Criticity
        ('LOW', _('Low')),
        # Translators: Criticity      
        ('MED', _('Medium')),
        # Translators: Criticity   
        ('HIG', _('High')),
        # Translators: Criticity     
        ('CRI', _('Critical')), 
    )
    STATUS_CHOICES = (
        # Translators: Ticket status
        ('OPEN', _('Open')),
        # Translators: Ticket status
        ('CLOSED', _('Close')),
    )
    id = models.AutoField(primary_key=True)
    # Translators: Ticket poster
    who = models.CharField(_('Origin'), max_length=50)
    #playerID = models.IntegerField(blank=True, editable=False)
    title = models.CharField(_('Title'), max_length=100)
    where = models.CharField(_('Location'), max_length=100)
    what = models.TextField(_('Description'))
    severity = models.CharField(_('Severity'), max_length=3, choices=SEVERITY_CHOICES)
    status = models.CharField(_('Status'), max_length=6, choices=STATUS_CHOICES, default="OPEN")
    
    #def get_absolute_url(self):
    #    return '/ingame/view_ticket.html?%d' % self.id     