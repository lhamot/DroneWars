
if(!Blockly.Language) Blockly.Language = {};



//******************   PLANET    **********************************************
Blockly.Language.dronewars_is_planet_free =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendInput(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_1,
		Blockly.INPUT_VALUE, 'PLANET', 'Planet');
		this.appendInput(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_2,
		Blockly.DUMMY_INPUT);
		this.setOutput(true, Boolean);
	}
};

Blockly.lua.dronewars_is_planet_free = function()
{
	var planet = Blockly.lua.valueToCode(
	               this, 'PLANET', Blockly.JavaScript.ORDER_NONE) || 'nil';
	return [ planet + ':isFree()', Blockly.lua.ORDER_FUNCTION_CALL];
};


addProperty('Planet', 'coord', 'Coord',
            Blockly.LANG_DRONEWARS_PLANET_COORD,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
addProperty('Planet', 'playerId', Number,
            Blockly.LANG_DRONEWARS_PLANET_PLAYERID,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
addProperty('Planet', 'buildingList', Array,
            Blockly.LANG_DRONEWARS_PLANET_BUILDINGLIST,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
addProperty('Planet', 'cannonTab', Array,
            Blockly.LANG_DRONEWARS_PLANET_CANNONLIST,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
addProperty('Planet', 'ressourceSet', 'RessourceSet',
            Blockly.LANG_DRONEWARS_PLANET_RESSOURCESET,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
//addProperty('Planet', 'isFree', Boolean,
//            Blockly.LANG_DRONEWARS_PLANET_IS_FREE);

// BuildingList
Blockly.Language.dronewars_building_in_planet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
	function()
	{
	  var buildingDropdown = getBuildingDropdown();

		this.setColour(230);
		//this.setInputsInline(true);
		this.appendTitle(buildingDropdown, 'BUILDING');
		this.appendInput(Blockly.LANG_DRONEWARS_BUILDING_IN_PLANET,
		Blockly.INPUT_VALUE, 'PLANET', 'Planet');
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_building_in_planet = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
	return [object + '.buildingList[Building.' + this.getTitleValue('BUILDING') + ']',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

//CannonList
Blockly.Language.dronewars_cannon_in_planet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
	function()
	{
		this.setColour(230);
		//this.setInputsInline(true);
		this.appendTitle(getCannonDropdown(), 'CANNON');
		this.appendInput(Blockly.LANG_DRONEWARS_CANNON_IN_PLANET,
		Blockly.INPUT_VALUE, 'PLANET', 'Planet');
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_cannon_in_planet = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
	return [object + '.cannonTab[Cannon.' + this.getTitleValue('CANNON') + ']',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

//RessourceSet
Blockly.Language.dronewars_ressource_in_planet =
{
  category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
  init:
    function () {
      var buildingDropdown = getRessourceDropDown();

      this.setColour(230);
      //this.setInputsInline(true);
      this.appendTitle(buildingDropdown, 'RESSOURCE');
      this.appendInput(Blockly.LANG_DRONEWARS_RESSOURCE_IN_PLANET,
      Blockly.INPUT_VALUE, 'PLANET', 'Planet');
      this.setOutput(true, Number);
    }
};

Blockly.lua.dronewars_ressource_in_planet = function () {
  var object = Blockly.lua.valueToCode(
	               this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
  return [object + '.ressourceSet:at(Ressource.' + this.getTitleValue('RESSOURCE') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
  ];
};