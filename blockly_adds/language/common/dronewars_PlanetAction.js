
if(!Blockly.Language) Blockly.Language = {};


//******************   ACTION    **********************************************
Blockly.Language.dronewars_makeBuilding =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
	  var buildingDropdown = getBuildingDropdown();

		this.setColour(230);
		this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_MAKE_BUILDING)
      .appendTitle(buildingDropdown, 'BUILDING');
		this.setOutput(true, 'PlanetAction');
	}
};


Blockly.lua.dronewars_makeBuilding = function ()
{
	return ['makeBuilding(Building.' + this.getTitleValue('BUILDING') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


Blockly.Language.dronewars_makeShip =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
	  this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_MAKE_SHIP)
      .appendTitle(getShipDropDown(), 'SHIP');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_makeShip = function()
{
	return ['makeShip(Ship.' + this.getTitleValue('SHIP') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


Blockly.Language.dronewars_makeCannon =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
		this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_MAKE_CANNON)
      .appendTitle(getCannonDropdown(), 'CANNON');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_makeCannon = function()
{
	return ['makeCannon(Cannon.' + this.getTitleValue('CANNON') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

Blockly.Language.dronewars_doNothing =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
		this.setColour(230);
	  this.appendDummyInput()
        .appendTitle(Blockly.LANG_DRONEWARS_PLANET_DO_NOTHING);
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_doNothing = function()
{
	return ['noPlanetAction()', Blockly.lua.ORDER_FUNCTION_CALL];
};

