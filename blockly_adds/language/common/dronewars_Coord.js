
if(!Blockly.Language) Blockly.Language = {};


//******************   COORD    ***********************************************
Blockly.Language.dronewars_coord =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_DRONEWARS_COORD);
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
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_DRONEWARS_RANDDIR);
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
	Blockly.LANG_CATEGORY_DRONEWARS_COORD,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendInput(Blockly.INPUT_VALUE, 'FROM', 'Coord').appendTitle(
      Blockly.LANG_DRONEWARS_DIR_FROM_TO_1);
		this.appendInput(Blockly.INPUT_VALUE, 'TO', 'Coord').appendTitle(
      Blockly.LANG_DRONEWARS_DIR_FROM_TO_2);
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

