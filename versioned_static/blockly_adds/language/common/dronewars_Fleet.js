'use strict';

goog.provide('Blockly.Lua');

//******************   Fleet    **********************************************
addProperty('Fleet', 'id', 'Number', Blockly.LANG_DRONEWARS_FLEET_ID,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'playerId', 'Number', Blockly.LANG_DRONEWARS_FLEET_PLAYER_ID,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'coord', 'Coord', Blockly.LANG_DRONEWARS_FLEET_COORD,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'origin', 'Coord', Blockly.LANG_DRONEWARS_FLEET_ORIGIN,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'name', 'String', Blockly.LANG_DRONEWARS_FLEET_NAME,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'shipList', 'Array', Blockly.LANG_DRONEWARS_FLEET_SHIPLIST,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'ressourceSet', 'RessourceSet', Blockly.LANG_DRONEWARS_FLEET_RESSOURCESET,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'memory', 'UserData', Blockly.LANG_DRONEWARS_FLEET_MEMORY,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'player', 'Player', Blockly.LANG_DRONEWARS_FLEET_PLAYER,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'task', 'FleetTask', Blockly.LANG_DRONEWARS_FLEET_TASK,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);

//ShipList
Blockly.Blocks['dronewars_ship_in_fleet'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput()
            .appendField(getShipDropDown(), 'SHIP');
        this.appendValueInput('FLEET')
            .setCheck('Fleet')
            .appendField(Blockly.LANG_DRONEWARS_SHIP_IN_FLEET);
		this.setOutput(true, 'Number');
	}
};

Blockly.Lua['dronewars_ship_in_fleet'] = function(block)
{
	var object = Blockly.Lua.valueToCode(
	               block, 'FLEET', Blockly.Lua.ORDER_NONE) || 'nil';
	return [object + '.shipList[Ship.' + block.getFieldValue('SHIP') + ']',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

//ShipList
Blockly.Blocks['dronewars_ressource_in_fleet'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
    function () {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput()
            .appendField(getRessourceDropDown(), 'RESSOURCE');
        this.appendValueInput('FLEET')
            .setCheck('Fleet')
            .appendField(Blockly.LANG_DRONEWARS_RESSOURCE_IN_FLEET);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_ressource_in_fleet'] = function (block) {
  var object = Blockly.Lua.valueToCode(
	               block, 'FLEET', Blockly.Lua.ORDER_NONE) || 'nil';
  return [object + '.ressourceSet:at(Ressource.' + block.getFieldValue('RESSOURCE') + ')',
	        Blockly.Lua.ORDER_HIGH
  ];
};


//ShipPrice
Blockly.Blocks['dronewars_ship_price'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
    function()
    {
        this.setColour(230);
        this.appendDummyInput()
            .appendField(Blockly.LANG_DRONEWARS_SHIP_PRICE)
            .appendField(getShipDropDown(), 'SHIP');
        this.setOutput(true, 'RessourceSet');
    }
};

Blockly.Lua['dronewars_ship_price'] = function(block)
{
    return ['shipPrice(Ship.' + block.getFieldValue('SHIP') + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};


//Age
Blockly.Blocks['dronewars_fleet_age'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('FLEET')
            .setCheck('Fleet')
            .appendField(Blockly.LANG_DRONEWARS_FLEET_AGE);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_fleet_age'] = function(block)
{
    var fleet = 
        Blockly.Lua.valueToCode(block, 'FLEET', Blockly.Lua.ORDER_NONE) || 'nil';
    return [fleet + ':age()', Blockly.Lua.ORDER_HIGH];
};

//simulates
Blockly.Blocks['dronewars_simulates'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('FLEET')
            .setCheck('Fleet')
            .appendField(Blockly.LANG_DRONEWARS_SIMULATE);
        this.appendValueInput('PLANET')
            .setCheck('Planet')
            .appendField(Blockly.LANG_DRONEWARS_SIMULATE_ARG2);
        this.appendValueInput('OTHER_FLEETS')
            .setCheck('Array')
            .appendField(Blockly.LANG_DRONEWARS_SIMULATE_ARG3);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_simulates'] = function(block)
{
    var fleet = Blockly.Lua.valueToCode(block, 'FLEET', Blockly.Lua.ORDER_NONE) || 'nil';
    var planet = Blockly.Lua.valueToCode(block, 'PLANET', Blockly.Lua.ORDER_NONE) || 'nil';
    var fleets = Blockly.Lua.valueToCode(block, 'OTHER_FLEETS', Blockly.Lua.ORDER_NONE) || 'nil';
    return ['simulates(' + fleet + ',' + planet + ',' + fleets + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};
