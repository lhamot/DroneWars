'use strict';

goog.provide('Blockly.Lua');

//******************   COORD    ***********************************************
Blockly.Blocks['dronewars_coord'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_COORD)
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'X')
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'Y')
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'Z');
		this.setOutput(true, 'Coord');
	}
};

Blockly.Lua['dronewars_coord'] = function(block)
{
	return ['Coord(' + block.getFieldValue('X') + ',' +
	        block.getFieldValue('Y') + ',' +
	        block.getFieldValue('Z') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

Blockly.Blocks['dronewars_randDir'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.appendDummyInput().appendField(Blockly.LANG_DRONEWARS_RANDDIR);
		this.setOutput(true, 'Direction');
	}
};

Blockly.Lua['dronewars_randDir'] = function(block)
{
	return ['directionRandom()',
	        Blockly.Lua.ORDER_HIGH
	       ];
};


Blockly.Blocks['dronewars_dir_from_to'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
	  this.appendValueInput('FROM')
      .setCheck('Coord')
      .appendField(Blockly.LANG_DRONEWARS_DIR_FROM_TO_1);
	  this.appendValueInput('TO')
      .setCheck('Coord')
      .appendField(Blockly.LANG_DRONEWARS_DIR_FROM_TO_2);
		this.setOutput(true, 'Direction');
	}
};

Blockly.Lua['dronewars_dir_from_to'] = function(block)
{
	var from = Blockly.Lua.valueToCode(block, 'FROM', Blockly.Lua.ORDER_NONE) || 'Coord()';
	var to = Blockly.Lua.valueToCode(block, 'TO', Blockly.Lua.ORDER_NONE) || 'Coord()';

	return ['directionFromTo(' + from + ',' + to + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

