'use strict';

goog.provide('Blockly.Lua');

//******************   PLANET    **********************************************
Blockly.Blocks['dronewars_is_planet_free'] =
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
      .appendField(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_1);
	  this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_IS_PLANET_FREE_2);
		this.setOutput(true, Boolean);
	}
};

Blockly.Lua['dronewars_is_planet_free'] = function(block)
{
	var planet = Blockly.Lua.valueToCode(
	               block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
	return [ planet + ':isFree()', Blockly.Lua.ORDER_HIGH];
};


Blockly.Blocks['dronewars_planet_age'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('PLANET')
            .setCheck('Planet')
            .appendField(Blockly.LANG_DRONEWARS_PLANET_AGE);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_planet_age'] = function(block)
{
    var planet = 
        Blockly.Lua.valueToCode(block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
    return [planet + ':age()', Blockly.Lua.ORDER_HIGH];
};


addProperty('Planet', 'coord', 'Coord',
            Blockly.LANG_DRONEWARS_PLANET_COORD,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET);
addProperty('Planet', 'playerId', 'Number',
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
Blockly.Blocks['dronewars_building_in_planet'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
	function()
	{
	  var buildingDropdown = getBuildingDropdown();

		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput().appendField(buildingDropdown, 'BUILDING');
	  this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendField(Blockly.LANG_DRONEWARS_BUILDING_IN_PLANET);
		this.setOutput(true, 'Number');
	}
};

Blockly.Lua['dronewars_building_in_planet'] = function(block)
{
	var object = Blockly.Lua.valueToCode(
	               block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
	return [object + '.buildingList[Building.' + block.getFieldValue('BUILDING') + ']',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

//CannonList
Blockly.Blocks['dronewars_cannon_in_planet'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput()
		  .appendField(getCannonDropdown(), 'CANNON');
	  this.appendValueInput('PLANET')
      .setCheck('Planet')
      .appendField(Blockly.LANG_DRONEWARS_CANNON_IN_PLANET);
		this.setOutput(true, 'Number');
	}
};

Blockly.Lua['dronewars_cannon_in_planet'] = function(block)
{
	var object = Blockly.Lua.valueToCode(
	               block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
	return [object + '.cannonTab[Cannon.' + block.getFieldValue('CANNON') + ']',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

//RessourceSet
Blockly.Blocks['dronewars_ressource_in_planet'] =
{
  category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
  init:
    function () {
      var buildingDropdown = getRessourceDropDown();

      this.setColour(230);
      this.setInputsInline(true);
      this.appendDummyInput()
        .appendField(buildingDropdown, 'RESSOURCE');
      this.appendValueInput('PLANET')
        .setCheck('Planet')
        .appendField(Blockly.LANG_DRONEWARS_RESSOURCE_IN_PLANET);
      this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_ressource_in_planet'] = function (block) {
  var object = Blockly.Lua.valueToCode(
	               block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
  return [object + '.ressourceSet:at(Ressource.' + block.getFieldValue('RESSOURCE') + ')',
	        Blockly.Lua.ORDER_HIGH
  ];
};


//CannonPrice
Blockly.Blocks['dronewars_cannon_price'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
      this.setColour(230);
      this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_CANNON_PRICE)
      .appendField(getCannonDropdown(), 'CANNON');
      this.setOutput(true, 'RessourceSet');
    }
};

Blockly.Lua['dronewars_cannon_price'] = function(block)
{
    return ['cannonPrice(Cannon.' + block.getFieldValue('CANNON') + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};

//BuildingPrice
Blockly.Blocks['dronewars_building_price'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLANET,
init:
    function()
    {
      this.setColour(230);
      this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_BUILDING_PRICE)
      .appendField(getBuildingDropdown(), 'BUILDING');
      this.appendValueInput("LEVEL")
      .appendField(Blockly.LANG_DRONEWARS_BUILDING_PRICE2)
      .setCheck('Number');
      this.setOutput(true, 'RessourceSet');
      this.setInputsInline(true);
    }
};

Blockly.Lua['dronewars_building_price'] = function(block)
{
  var level = Blockly.Lua.valueToCode(
                   block, 'LEVEL', Blockly.Lua.ORDER_NONE) || '1';    
    return ['buildingPrice(Building.' + block.getFieldValue('BUILDING') + ', ' + level + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};


//Planet can pay
Blockly.Blocks['dronewars_planet_can_play'] =
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
      .appendField(Blockly.LANG_DRONEWARS_PLANET_CAN_PAY_1);
      this.appendValueInput('PRICE')
      .setCheck('RessourceSet')
      .appendField(Blockly.LANG_DRONEWARS_PLANET_CAN_PAY_2);
      this.setOutput(true, Boolean);
    }
};

Blockly.Lua['dronewars_planet_can_play'] = function(block)
{
    var planet = Blockly.Lua.valueToCode(
                   block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
    var price = Blockly.Lua.valueToCode(
                   block, 'PRICE', Blockly.Lua.ORDER_NONE) || 'RessourceSet()';
    return [ planet + '.ressourceSet:contains(' + price + ')', Blockly.Lua.ORDER_HIGH];
};

