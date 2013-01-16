from django.utils.translation import ugettext_lazy as _
from django import template
from gen_py.thrift.ttypes import *
import datetime

register = template.Library()

@register.filter
def shipname(value):
    return  _(Ship_Enum._VALUES_TO_NAMES[value])

@register.filter
def eventname(value):
    return  _(Event_Type._VALUES_TO_NAMES[value])

@register.filter
def buildingname(value):
    return  _(Building_Enum._VALUES_TO_NAMES[value])


@register.filter
def cannonname(value):
    return  _(Cannon_Enum._VALUES_TO_NAMES[value])

@register.filter
def planettaskname(value):
    return  _(PlanetTask_Enum._VALUES_TO_NAMES[value])

@register.filter
def planettasktarget(task):
    if task.type == PlanetTask_Enum.UpgradeBuilding:
        return _(Building_Enum._VALUES_TO_NAMES[task.value])
    elif task.type == PlanetTask_Enum.MakeShip:
        return _(Ship_Enum._VALUES_TO_NAMES[task.value])
    elif task.type == PlanetTask_Enum.MakeCannon:
        return _(Cannon_Enum._VALUES_TO_NAMES[task.value])
    else:
        raise AssertionError("Unconsistent PlanetTask")
    

@register.tag
def forlevel(mode, level, planetmin, fleetmin):
    if mode == "Planet":
        if level >= planetmin:
            return True
        else:
            return False
    elif mode == "Fleet":
        if level >= fleetmin:
            return True
        else:
            return False
    else:
        raise AssertionError("Unexpected type")


@register.filter
def int2datetime(ts):
    return datetime.datetime.fromtimestamp(ts)
