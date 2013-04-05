taille_flottes = nil
my_fleet = nil
autre_flotte = nil
planete_locale = nil

taille_flottes2 = 10

-- Cette procedure permet à un capitaine de décider si sa flotte doit fusioner avec une autre flotte allié présente dans la mème zone.
-- En entrée elle recois votre flote "ma_flotte" et la flotte allié "autre_flotte", ce qui doit lui permetre de décider si il vaut mieu fusioner ou non.
function AI_do_gather(my_fleet2, autre_flotte2)
  --return (my_fleet2.shipList[Ship.Mosquito] + autre_flotte2.shipList[Ship.Mosquito]) <= 5
  --return (my_fleet2.shipList[Ship.Queen] + autre_flotte2.shipList[Ship.Queen]) <= 1
  return true
end
-- Cette procedure permet a un capitaine de décider si sa flotte doit combatre ou fuire.
-- En entrée elle recois votre flote "ma_flotte" et la flotte enemie "autre_flotte", ce qui doit lui permetre de décider si il vaut mieu combatre ou fuire.
function AI_do_fight(my_fleet2, autre_flotte2)
  return true
end
-- Cette procédure décrit le comportment d'une flotte à chaque tour.
-- Elle recois deux variables :
-- Une "ma_flotte" qui représente la flotte dont c'est le tour de jouer, et une variable "planètes" qui peut contenir une planète si il y en as une dans la mème zone.
-- Cette procedure retourne la variable "ordre" qui contient l'ordre à donner aux vaisseaux de la flotte.
function AI_action(my_fleet2, planete_locale2)
  if planete_locale2 then
    if planete_locale2:isFree() then
      if(my_fleet2.shipList[Ship.Queen] > 0) then 
        if(math.random(2) == 1) then return FleetAction(FleetAction.Colonize,Coord()) end
      end
      if(planete_locale2.ressourceSet:at(Ressource.Metal) > 0) then
        return FleetAction(FleetAction.Harvest,Coord())
      end
    elseif my_fleet2.coord == my_fleet2.origin then
      if(my_fleet2.ressourceSet:at(Ressource.Metal) > 0) then
        return FleetAction(FleetAction.Drop,Coord())
      end
      if(my_fleet2.shipList[Ship.Queen] < 1) then
        return FleetAction(FleetAction.Nothing,Coord())
      end
    end
  end
  if(my_fleet2.ressourceSet:at(Ressource.Metal) > (10000)) then
    return FleetAction(FleetAction.Move,directionFromTo(my_fleet2.coord, my_fleet2.origin))
  end
  return FleetAction(FleetAction.Move,directionRandom())
end


