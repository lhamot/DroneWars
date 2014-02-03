'use strict';

goog.provide('Blockly.Lua');

//******************   ACTION    **********************************************
Blockly.Blocks['dronewars_makeBuilding'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
	  var buildingDropdown = getBuildingDropdown();

		this.setColour(230);
		this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_MAKE_BUILDING)
      .appendField(buildingDropdown, 'BUILDING');
		this.setOutput(true, 'PlanetAction');
	}
};


Blockly.Lua['dronewars_makeBuilding'] = function (block)
{
	return ['makeBuilding(Building.' + block.getFieldValue('BUILDING') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};


Blockly.Blocks['dronewars_makeShip'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
	  this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_MAKE_SHIP)
      .appendField(getShipDropDown(), 'SHIP');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.Lua['dronewars_makeShip'] = function(block)
{
	return ['makeShip(Ship.' + block.getFieldValue('SHIP') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};


Blockly.Blocks['dronewars_makeCannon'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
		this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_MAKE_CANNON)
      .appendField(getCannonDropdown(), 'CANNON');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.Lua['dronewars_makeCannon'] = function(block)
{
	return ['makeCannon(Cannon.' + block.getFieldValue('CANNON') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

Blockly.Blocks['dronewars_doNothing'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
	  this.appendDummyInput()
        .appendField(Blockly.LANG_DRONEWARS_PLANET_DO_NOTHING);
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.Lua['dronewars_doNothing'] = function(block)
{
	return ['noPlanetAction()', Blockly.Lua.ORDER_HIGH];
};

