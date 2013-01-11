
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
	  this.appendValueInput('COORD')
      .setCheck('Coord')
      .appendTitle(Blockly.LANG_DRONEWARS_FLEETACTION_CTOR_2);
		this.setOutput(true, 'FleetAction');
	}
};

Blockly.lua.dronewars_fleetaction = function()
{
	var coord = Blockly.lua.valueToCode(
	              this, 'COORD', Blockly.lua.ORDER_NONE) || 'Coord()';
	return ['FleetAction(FleetAction.' + this.getTitleValue('ACTION') + ',' +
	        coord + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};