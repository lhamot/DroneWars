'use strict';

goog.provide('Blockly.Lua');

//********************** FleetAction ******************************************
Blockly.Blocks['dronewars_fleetaction'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_1);
		this.appendDummyInput().appendField(getFleetActionDropdown(), 'ACTION');
	  this.appendValueInput('DIREC')
      .setCheck('Direction')
      .appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_2);
		this.setOutput(true, 'FleetAction');
	}
};

Blockly.Lua['dronewars_fleetaction'] = function(block)
{
	var coord = Blockly.Lua.valueToCode(
	              block, 'DIREC', Blockly.Lua.ORDER_NONE) || 'Direction()';
	return ['FleetAction(FleetAction.' + block.getFieldValue('ACTION') + ',' +
	        coord + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};


//********************** FleetAction - Nothing ******************************************
Blockly.Blocks['dronewars_fleetDoNothing'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_DO_NOTHING);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.Lua['dronewars_fleetDoNothing'] = function(block)
{
    return ['FleetAction(FleetAction.Nothing, Direction())',
            Blockly.Lua.ORDER_HIGH
           ];
};


//********************** FleetAction - Move ******************************************
Blockly.Blocks['dronewars_fleetMove'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_MOVE);
        this.appendValueInput('DIREC')
            .setCheck('Direction')
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.Lua['dronewars_fleetMove'] = function(block)
{
    var coord = Blockly.Lua.valueToCode(
                  block, 'DIREC', Blockly.Lua.ORDER_NONE) || 'Direction()';
    return ['FleetAction(FleetAction.Move,' + coord + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};


//********************** FleetAction - Harvest ******************************************
Blockly.Blocks['dronewars_fleetHarvest'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_HARVEST);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.Lua['dronewars_fleetHarvest'] = function(block)
{
    return ['FleetAction(FleetAction.Harvest, Coord())',
            Blockly.Lua.ORDER_HIGH
           ];
};


//********************** FleetAction - Colonize ******************************************
Blockly.Blocks['dronewars_fleetColonize'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_COLONIZE);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.Lua['dronewars_fleetColonize'] = function(block)
{
    return ['FleetAction(FleetAction.Colonize, Coord())',
            Blockly.Lua.ORDER_HIGH
           ];
};


//********************** FleetAction - Drop ******************************************
Blockly.Blocks['dronewars_fleetDrop'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_DROP);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.Lua['dronewars_fleetDrop'] = function(block)
{
    return ['FleetAction(FleetAction.Drop, Coord())',
            Blockly.Lua.ORDER_HIGH
           ];
};
