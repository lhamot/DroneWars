taille_flottes = nil
my_fleet = nil
autre_flotte = nil
planete_locale = nil

taille_flottes2 = 10

function AI_do_gather(my_fleet2, autre_flotte2)
  return true
end
function AI_do_fight(my_fleet2, autre_flotte2)
  return true
end
function AI_action(my_fleet2, planete_locale2)
  if planete_locale2 then
    if planete_locale2:isFree() then
      if(my_fleet2.shipList[Ship.Queen] > 0) then 
        if(math.random(2) == 1) then return FleetAction(FleetAction.Colonize) end
      end
      if(planete_locale2.ressourceSet:at(Ressource.Metal) > 0) then
        return FleetAction(FleetAction.Harvest)
      end
    elseif my_fleet2.coord == my_fleet2.origin then
      if(my_fleet2.ressourceSet:at(Ressource.Metal) > 0) then
        return FleetAction(FleetAction.Drop)
      end
      if(my_fleet2.shipList[Ship.Queen] < 1) then
        return FleetAction(FleetAction.Nothing)
      end
    end
  end
  if(my_fleet2.ressourceSet:at(Ressource.Metal) > (10000)) then
    return FleetAction(FleetAction.Move,directionFromTo(my_fleet2.coord, my_fleet2.origin))
  end
  return FleetAction(FleetAction.Move,directionRandom())
end


