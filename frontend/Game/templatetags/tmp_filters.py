from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ugettext_lazy as N_
from django import template
from gen_py.thrift.ttypes import *
import datetime
from Game.views import Ship_Enum

register = template.Library()


@register.filter
def shipname(value):
    return  _(Ship_Enum._VALUES_TO_NAMES[value])


@register.filter
def shipbrief(value):
    return  N_("SHIP_BRIEF_%i" % (value))


@register.filter
def eventname(value):
    return  _(Event_Type._VALUES_TO_NAMES[value])


@register.filter
def eventvalue(event):
    if event.value == -1:
        return ""
    if event.type == Event_Type.CannonMade:
        return  _(Cannon_Enum._VALUES_TO_NAMES[event.value])
    elif event.type == Event_Type.ShipMade:
        return  _(Ship_Enum._VALUES_TO_NAMES[event.value])
    elif event.type == Event_Type.Upgraded:
        return  _(Building_Enum._VALUES_TO_NAMES[event.value])
    else:
        return ""


@register.filter
def buildingname(value):
    return  _(Building_Enum._VALUES_TO_NAMES[value])

@register.filter
def buildingbrief(value):
    return  N_("BUILDING_BRIEF_%i" % (value))

@register.filter
def cannonname(value):
    return  _(Cannon_Enum._VALUES_TO_NAMES[value])

@register.filter
def cannonbrief(value):
    return  N_("CANNON_BRIEF_%i" % (value))

@register.filter
def planettaskname(value):
    return  _(PlanetTask_Enum._VALUES_TO_NAMES[value])


@register.filter
def fleettaskname(value):
    return  _(FleetTask_Enum._VALUES_TO_NAMES[value])


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
    

@register.filter
def fleettasktarget(task):
    return coord(task.position)


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
def coord(point):
    return _("(%i;%i;%i)") % (point.X, point.Y, point.Z); 


@register.filter
def content(planetOrFleet):
    if isinstance(planetOrFleet, Fleet):
        fleet = planetOrFleet
        if len(fleet.shipList) != Ship_Enum.Count:
            raise AssertionError("fleet.shipList.size() != Ship::Count");
        result = ""
        for shipType in range(Ship_Enum.Count):
            if fleet.shipList[shipType]:
                result += " " + _("%s:%s;") % (shipname(shipType)[0], fleet.shipList[shipType]);
        if len(result) > 0:
            result = result[:-1] 
        return result
    elif isinstance(planetOrFleet, Planet):
        planet = planetOrFleet
        if len(planet.cannonTab) != Cannon_Enum.Count:
            raise AssertionError("planet.cannonTab.size() != Cannon::Count");
        result = ""
        for cannonType in range(Cannon_Enum.Count):
            if planet.cannonTab[cannonType]:
                result += _("%s:%s;") % (cannonname(cannonType)[0], planet.cannonTab[cannonType]);
        if len(result) > 0:
            result = result[:-1] 
        return result
    else:
        raise AssertionError("Unexpected type")


@register.filter
def timeleft(task, timeinfo):
    end = task.lauchTime + task.duration
    roundLeft = end - timeinfo.univTime
    return roundLeft * timeinfo.roundDuration

    
@register.filter
def roundtimeleft(timeinfo):
    intDec = divmod(timeinfo.univTime, 1.)
    roundleft = timeinfo.roundDuration * (1. - intDec[1])
    return roundleft


@register.filter()
def img_path(planet, prefix):
    PlanetImageCount = 23
    planetHash = planet.coord.X + (planet.coord.Y * 1000) + (planet.coord.Z * 1000000) 
    return "img/Planetes/%s%03i.jpg" % (prefix, planetHash % PlanetImageCount);
