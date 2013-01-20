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

@register.filter
def coord(coord):
    return "(%(X);%(Y);%(Z))" % coord; 


@register.filter
def content(planetOrFleet):
    if isinstance(planetOrFleet, Fleet):
        fleet = planetOrFleet
        if len(fleet.shipList) != Ship_Enum.Count:
            raise AssertionError("fleet.shipList.size() != Ship::Count");
        result = ""
        for shipType in range(Ship_Enum.Count):
            if fleet.shipList[shipType]:
                result += shipname(shipType)[0] + ":" + str(fleet.shipList[shipType]) + ";";
        if len(result) > 0:
            result = result[:-1] 
        return result
    elif isinstance(planetOrFleet, Planet):
        pass
    else:
        raise AssertionError("Unexpected type")


@register.filter
def timeleft(task, timeinfo):
    end = task.lauchTime + task.duration
    roundLeft = end - timeinfo.univTime
    return roundLeft * timeinfo.roundDuration
    



