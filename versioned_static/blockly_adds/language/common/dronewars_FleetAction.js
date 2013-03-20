
if(!Blockly.Language) Blockly.Language = {};



//********************** FleetAction ******************************************
Blockly.Language.dronewars_fleetaction =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_1);
		this.appendDummyInput().appendTitle(getFleetActionDropdown(), 'ACTION');
	  this.appendValueInput('DIREC')
      .setCheck('Direction')
      .appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_2);
		this.setOutput(true, 'FleetAction');
	}
};

Blockly.lua.dronewars_fleetaction = function()
{
	var coord = Blockly.lua.valueToCode(
	              this, 'DIREC', Blockly.lua.ORDER_NONE) || 'Direction()';
	return ['FleetAction(FleetAction.' + this.getTitleValue('ACTION') + ',' +
	        coord + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


//********************** FleetAction - Nothing ******************************************
Blockly.Language.dronewars_fleetDoNothing =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_DO_NOTHING);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.lua.dronewars_fleetDoNothing = function()
{
    return ['FleetAction(FleetAction.Nothing, Coord())',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};


//********************** FleetAction - Move ******************************************
Blockly.Language.dronewars_fleetMove =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_MOVE);
        this.appendValueInput('DIREC')
            .setCheck('Direction')
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.lua.dronewars_fleetMove = function()
{
    var coord = Blockly.lua.valueToCode(
                  this, 'DIREC', Blockly.lua.ORDER_NONE) || 'Direction()';
    return ['FleetAction(FleetAction.Move,' + coord + ')',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};


//********************** FleetAction - Harvest ******************************************
Blockly.Language.dronewars_fleetHarvest =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_HARVEST);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.lua.dronewars_fleetHarvest = function()
{
    return ['FleetAction(FleetAction.Harvest, Coord())',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};


//********************** FleetAction - Colonize ******************************************
Blockly.Language.dronewars_fleetColonize =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_COLONIZE);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.lua.dronewars_fleetColonize = function()
{
    return ['FleetAction(FleetAction.Colonize, Coord())',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};


//********************** FleetAction - Drop ******************************************
Blockly.Language.dronewars_fleetDrop =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEETACTION,
init:
    function()
    {
        this.setColour(230);
        this.setInputsInline(true);
        this.appendDummyInput().appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_DROP);
        this.setOutput(true, 'FleetAction');
    }
};

Blockly.lua.dronewars_fleetDrop = function()
{
    return ['FleetAction(FleetAction.Drop, Coord())',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};
