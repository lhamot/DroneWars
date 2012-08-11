
if(!Blockly.Language) Blockly.Language = {};

function checkNumber(text)
{
	var n = window.parseFloat(text || 0);
	return window.isNaN(n) ? null : String(n);
}

function getShipDropDown()
{
	return new Blockly.FieldDropdown([
	                                   ['Mosquito', 'Mosquito'],
	                                   ['Hornet', 'Hornet'],
	                                   ['Vulture', 'Vulture'],
	                                   ['Dragon', 'Dragon'],
	                                   ['Behemoth', 'Behemoth'],
	                                   ['Azathoth', 'Azathoth'],
	                                   ['Queen', 'Queen'],
	                                   ['Cargo', 'Cargo'],
	                                   ['LargeCargo', 'LargeCargo']
	                                 ]);
}

function getCannonDropdown()
{
	return new Blockly.FieldDropdown([
	                                   ['Cannon1', 'Cannon1'],
	                                   ['Cannon2', 'Cannon2'],
	                                   ['Cannon3', 'Cannon3'],
	                                   ['Cannon4', 'Cannon4'],
	                                   ['Cannon5', 'Cannon5'],
	                                   ['Cannon6', 'Cannon6']
	                                 ]);
}

function getFleetActionDropdown()
{
	return new Blockly.FieldDropdown([
	                                   ['Nothing',  'Nothing'],
	                                   ['Move',     'Move'],
	                                   ['Harvest',  'Harvest'],
	                                   ['Colonize', 'Colonize'],
	                                   ['Drop',     'Drop'],
	                                 ]);
}

//******************   COORD    ***********************************************
Blockly.Language.dronewars_coord =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_COORD);
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'X');
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'Y');
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'Z');
		this.setOutput(true, 'Coord');
	}
};

Blockly.lua.dronewars_coord = function()
{
	return ['Coord(' + this.getTitleValue('X') + ',' +
	        this.getTitleValue('Y') + ',' +
	        this.getTitleValue('Z') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

Blockly.Language.dronewars_randDir =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_RANDDIR);
		this.setOutput(true, 'Coord');
	}
};

Blockly.lua.dronewars_randDir = function()
{
	return ['directionRandom()',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


Blockly.Language.dronewars_dir_from_to =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_DIR_FROM_TO_1,
		Blockly.INPUT_VALUE, 'FROM', 'Coord');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_DIR_FROM_TO_2,
		Blockly.INPUT_VALUE, 'TO', 'Coord');
		this.setOutput(true, 'Coord');
	}
};

Blockly.lua.dronewars_dir_from_to = function()
{
	var from = Blockly.lua.valueToCode(this, 'FROM', Blockly.JavaScript.ORDER_NONE) || 'Coord()';
	var to = Blockly.lua.valueToCode(this, 'TO', Blockly.JavaScript.ORDER_NONE) || 'Coord()';

	return ['directionFromTo(' + from + ',' + to + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

//******************   ACTION    **********************************************
Blockly.Language.dronewars_makeBuilding =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		var buildingDropdown = new Blockly.FieldDropdown([
		  ['CommandCenter', 'CommandCenter'],
		  ['MetalMine', 'MetalMine'],
		  ['CarbonMine', 'CarbonMine'],
		  ['LoiciumFilter', 'LoiciumFilter'],
		  ['Factory', 'Factory'],
		  ['Laboratory', 'Laboratory'],
		  ['CarbonicCentral', 'CarbonicCentral'],
		  ['SolarCentral', 'SolarCentral'],
		  ['GeothermicCentral', 'GeothermicCentral']
		]);

		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_MAKE_BUILDING);
		this.appendTitle(buildingDropdown, 'BUILDING');
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_makeBuilding = function()
{
	return ['makeBuilding(Building.' + this.getTitleValue('BUILDING') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


Blockly.Language.dronewars_makeShip =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_MAKE_SHIP);
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
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_MAKE_CANNON);
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
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_DO_NOTHING);
		this.setOutput(true, 'PlanetAction');
	}
};

Blockly.lua.dronewars_doNothing = function()
{
	return ['noPlanetAction()', Blockly.lua.ORDER_FUNCTION_CALL];
};

//******************   PLANET    **********************************************
Blockly.Language.dronewars_is_planet_free =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_IS_PLANET_FREE_1,
		Blockly.INPUT_VALUE, 'PLANET', 'Planet');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_IS_PLANET_FREE_2,
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

function addProperty(className, propName, returnType, label)
{
	jsFuncName = 'dronewars_' + className + '_' + propName;
	upperClassName = className.toUpperCase();
	Blockly.Language[jsFuncName] =
	{
category:
		Blockly.LANG_CATEGORY_DRONEWARS,
init:
		function()
		{
			this.setColour(230);
			//this.setInputsInline(true);
			this.appendInput(label,
			Blockly.INPUT_VALUE, upperClassName, className);
			this.setOutput(true, returnType);
		}
	};

	Blockly.lua[jsFuncName] = function()
	{
		var object = Blockly.lua.valueToCode(
		               this, upperClassName, Blockly.JavaScript.ORDER_NONE) || 'nil';
		return [object + '.' + propName, Blockly.lua.ORDER_FUNCTION_CALL];
	};
}

addProperty('Planet', 'coord', 'Coord',
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_COORD);
addProperty('Planet', 'playerId', Number,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_PLAYERID);
addProperty('Planet', 'buildingList', Array,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_BUILDINGLIST);
addProperty('Planet', 'cannonTab', Array,
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_CANNONLIST);
addProperty('Planet', 'ressourceSet', 'RessourceSet',
            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_RESSOURCESET);
//addProperty('Planet', 'isFree', Boolean,
//            Blockly.LANG_CATEGORY_DRONEWARS_PLANET_IS_FREE);

// BuildingList
Blockly.Language.dronewars_building_in_planet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		var buildingDropdown = new Blockly.FieldDropdown([
		  ['CommandCenter', 'CommandCenter'],
		  ['MetalMine', 'MetalMine'],
		  ['CarbonMine', 'CarbonMine'],
		  ['LoiciumFilter', 'LoiciumFilter'],
		  ['Factory', 'Factory'],
		  ['Laboratory', 'Laboratory'],
		  ['CarbonicCentral', 'CarbonicCentral'],
		  ['SolarCentral', 'SolarCentral'],
		  ['GeothermicCentral', 'GeothermicCentral']
		]);

		this.setColour(230);
		//this.setInputsInline(true);
		this.appendTitle(buildingDropdown, 'BUILDING');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_BUILDING_IN_PLANET,
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
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		//this.setInputsInline(true);
		this.appendTitle(getCannonDropdown(), 'CANNON');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_CANNON_IN_PLANET,
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

//******************   Fleet    **********************************************
addProperty('Fleet', 'id', Number, Blockly.LANG_CATEGORY_DRONEWARS_FLEET_ID);
addProperty('Fleet', 'playerId', Number, Blockly.LANG_CATEGORY_DRONEWARS_FLEET_PLAYER_ID);
addProperty('Fleet', 'coord', 'Coord', Blockly.LANG_CATEGORY_DRONEWARS_FLEET_COORD);
addProperty('Fleet', 'origin', 'Coord', Blockly.LANG_CATEGORY_DRONEWARS_FLEET_ORIGIN);
addProperty('Fleet', 'name', String, Blockly.LANG_CATEGORY_DRONEWARS_FLEET_NAME);
addProperty('Fleet', 'shipList', Array, Blockly.LANG_CATEGORY_DRONEWARS_FLEET_SHIPLIST);
addProperty('Fleet', 'ressourceSet', 'RessourceSet', Blockly.LANG_CATEGORY_DRONEWARS_FLEET_RESSOURCESET);

//ShipList
Blockly.Language.dronewars_ship_in_fleet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		//this.setInputsInline(true);
		this.appendTitle(getShipDropDown(), 'SHIP');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_SHIP_IN_FLEET,
		Blockly.INPUT_VALUE, 'FLEET', 'Fleet');
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_ship_in_fleet = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'FLEET', Blockly.lua.ORDER_NONE) || 'nil';
	return [object + '.shipList[Ship.' + this.getTitleValue('SHIP') + ']',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

//********************* RessourceSet ******************************************
Blockly.Language.dronewars_ressourceSet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET);
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R1');
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R2');
		this.appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R3');
		this.setOutput(true, 'RessourceSet');
	}
};

Blockly.lua.dronewars_ressourceSet = function()
{
	return ['RessourceSet(' + this.getTitleValue('R1') + ',' +
	        this.getTitleValue('R2') + ',' +
	        this.getTitleValue('R3') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

Blockly.Language.dronewars_ressourceSet_at =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		//this.setInputsInline(true);
		var dropdown = new Blockly.FieldDropdown([
		  ['Metal', 'Metal'],
		  ['Carbon', 'Carbon'],
		  ['Loicium', 'Loicium']
		]);
		this.appendTitle(dropdown, 'RESSOURCE');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET_AT,
		Blockly.INPUT_VALUE, 'RESSOURCESET', 'RessourceSet');
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_ressourceSet_at = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'RESSOURCESET', Blockly.JavaScript.ORDER_NONE) || 'nil';
	return [object + ':at(Ressource.' + this.getTitleValue('RESSOURCE') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};



//********************** FleetAction ******************************************
Blockly.Language.dronewars_fleetaction =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendTitle(Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION_CTOR_1);
		this.appendTitle(getFleetActionDropdown(), 'ACTION');
		this.appendInput(Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION_CTOR_2,
		Blockly.INPUT_VALUE, 'COORD', 'Coord');
		this.setOutput(true, 'FleetAction');
	}
};

Blockly.lua.dronewars_fleetaction = function()
{
	var coord = Blockly.lua.valueToCode(
	              this, 'COORD', Blockly.JavaScript.ORDER_NONE) || 'Coord()';
	return ['FleetAction(FleetAction.' + this.getTitleValue('ACTION') + ',' +
	        coord + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};