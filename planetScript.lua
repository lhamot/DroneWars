ma_planete = nil
flottes = nil
function AI(ma_planete2, flottes2)
  if(ma_planete2.buildingList[Building.CommandCenter] < 5) then
    if(ma_planete2.buildingList[Building.CommandCenter] <= ma_planete2.buildingList[Building.Factory]) then
      return makeBuilding(Building.CommandCenter)
    end
    if(ma_planete2.buildingList[Building.MetalMine] < ma_planete2.buildingList[Building.CommandCenter]) then
      return makeBuilding(Building.MetalMine)
    end
    if(ma_planete2.buildingList[Building.Factory] < ma_planete2.buildingList[Building.CommandCenter]) then
      return makeBuilding(Building.Factory)
    end
  end

  if(ma_planete2.ressourceSet:at(Ressource.Metal) >= 2000) then
    if(math.random(10) == 1) then
      return makeShip(Ship.Queen)
    else
      return makeShip(Ship.Mosquito)
    end
  end
  return noPlanetAction()
end
