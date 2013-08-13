
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
	  this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendTitle(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_1);
	  this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_2);
		this.setOutput(true, Boolean);
	}
};

Blockly.lua.dronewars_is_planet_free = function()
{
	var planet = Blockly.lua.valueToCode(
	               this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
	return [ planet + ':isFree()', Blockly.lua.ORDER_FUNCTION_CALL];
};


Blockly.Language.dronewars_planet_age =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('PLANET')
            .setCheck('Planet')
            .appendTitle(Blockly.LANG_DRONEWARS_PLANET_AGE);
        this.setOutput(true, Number);
    }
};

Blockly.lua.dronewars_planet_age = function()
{
    var planet = 
        Blockly.lua.valueToCode(this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
    return [planet + ':age()', Blockly.lua.ORDER_FUNCTION_CALL];
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
		this.setInputsInline(true);
		this.appendDummyInput().appendTitle(buildingDropdown, 'BUILDING');
	  this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendTitle(Blockly.LANG_DRONEWARS_BUILDING_IN_PLANET);
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
		this.setInputsInline(true);
		this.appendDummyInput()
		  .appendTitle(getCannonDropdown(), 'CANNON');
	  this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendTitle(Blockly.LANG_DRONEWARS_CANNON_IN_PLANET);
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
      this.setInputsInline(true);
      this.appendDummyInput()
        .appendTitle(buildingDropdown, 'RESSOURCE');
      this.appendValueInput('PLANET')
        .setCheck('Planet')
        .appendTitle(Blockly.LANG_DRONEWARS_RESSOURCE_IN_PLANET);
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


//CannonPrice
Blockly.Language.dronewars_cannon_price =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
      this.setColour(230);
      this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_CANNON_PRICE)
      .appendTitle(getCannonDropdown(), 'CANNON');
      this.setOutput(true, 'RessourceSet');
    }
};

Blockly.lua.dronewars_cannon_price = function()
{
    return ['cannonPrice(Cannon.' + this.getTitleValue('CANNON') + ')',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};

//BuildingPrice
Blockly.Language.dronewars_building_price =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
      this.setColour(230);
      this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_BUILDING_PRICE)
      .appendTitle(getBuildingDropdown(), 'BUILDING');
      this.appendValueInput("LEVEL")
      .appendTitle(Blockly.LANG_DRONEWARS_BUILDING_PRICE2)
      .setCheck(Number);
      this.setOutput(true, 'RessourceSet');
      this.setInputsInline(true);
    }
};

Blockly.lua.dronewars_building_price = function()
{
  var level = Blockly.lua.valueToCode(
                   this, 'LEVEL', Blockly.lua.ORDER_NONE) || '1';    
    return ['buildingPrice(Building.' + this.getTitleValue('BUILDING') + ', ' + level + ')',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};


//Planet can pay
Blockly.Language.dronewars_planet_can_play =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
      this.setColour(230);
      this.setInputsInline(true);
      this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendTitle(Blockly.LANG_DRONEWARS_PLANET_CAN_PAY_1);
      this.appendValueInput('PRICE')
      .setCheck('RessourceSet')
      .appendTitle(Blockly.LANG_DRONEWARS_PLANET_CAN_PAY_2);
      this.setOutput(true, Boolean);
    }
};

Blockly.lua.dronewars_planet_can_play = function()
{
    var planet = Blockly.lua.valueToCode(
                   this, 'PLANET', Blockly.lua.ORDER_NONE) || 'nil';
    var price = Blockly.lua.valueToCode(
                   this, 'PRICE', Blockly.lua.ORDER_NONE) || 'RessourceSet()';
    return [ planet + '.ressourceSet:contains(' + price + ')', Blockly.lua.ORDER_FUNCTION_CALL];
};

