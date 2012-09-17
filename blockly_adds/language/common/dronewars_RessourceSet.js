
if(!Blockly.Language) Blockly.Language = {};


//********************* RessourceSet ******************************************
Blockly.Language.dronewars_ressourceSet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET,
init:
	function()
	{
		this.setColour(230);
		this.appendTitle(Blockly.LANG_DRONEWARS_RESSOURCESET);
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
	Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET,
init:
	function()
	{
		this.setColour(230);
		//this.setInputsInline(true);
		var dropdown = getRessourceDropDown();
		this.appendTitle(dropdown, 'RESSOURCE');
		this.appendInput(Blockly.LANG_DRONEWARS_RESSOURCESET_AT,
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


