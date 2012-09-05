
if(!Blockly.Language) Blockly.Language = {};


//******************   ACTION    **********************************************
Blockly.Language.dronewars_makeBuilding =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_PLANETACTION,
init:
	function()
	{
	  var buildingDropdown = new Blockly.FieldDropdown(this.BUILDINGS);

		this.setColour(230);
		this.appendTitle(Blockly.LANG_DRONEWARS_MAKE_BUILDING);
		this.appendTitle(buildingDropdown, 'BUILDING');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.Language.dronewars_makeBuilding.BUILDINGS = [
		  [Blockly.LANG_DRONEWARS_BUILDING1, 'CommandCenter'],
		  [Blockly.LANG_DRONEWARS_BUILDING2, 'MetalMine'],
		  [Blockly.LANG_DRONEWARS_BUILDING3, 'CarbonMine'],
		  [Blockly.LANG_DRONEWARS_BUILDING4, 'LoiciumFilter'],
		  [Blockly.LANG_DRONEWARS_BUILDING5, 'Factory'],
		  [Blockly.LANG_DRONEWARS_BUILDING6, 'Laboratory'],
		  [Blockly.LANG_DRONEWARS_BUILDING7, 'CarbonicCentral'],
		  [Blockly.LANG_DRONEWARS_BUILDING8, 'SolarCentral'],
		  [Blockly.LANG_DRONEWARS_BUILDING9, 'GeothermicCentral']
];

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
		this.appendTitle(Blockly.LANG_DRONEWARS_MAKE_SHIP);
		this.appendTitle(getShipDropDown(), 'SHIP');
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
		this.appendTitle(Blockly.LANG_DRONEWARS_MAKE_CANNON);
		this.appendTitle(getCannonDropdown(), 'CANNON');
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
		this.appendTitle(Blockly.LANG_DRONEWARS_PLANET_DO_NOTHING);
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_doNothing = function()
{
	return ['noPlanetAction()', Blockly.lua.ORDER_FUNCTION_CALL];
};

