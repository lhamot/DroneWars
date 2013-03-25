
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
		this.appendDummyInput()
      .appendTitle(Blockly.LANG_DRONEWARS_RESSOURCESET)
      .appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R1')
      .appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R2')
      .appendTitle(new Blockly.FieldTextInput('0', checkNumber), 'R3');
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
		this.setInputsInline(true);
		var dropdown = getRessourceDropDown();
		this.appendDummyInput()
      .appendTitle(dropdown, 'RESSOURCE');
	  this.appendValueInput('RESSOURCESET')
      .setCheck('RessourceSet')
      .appendTitle(Blockly.LANG_DRONEWARS_RESSOURCESET_AT);
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_ressourceSet_at = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'RESSOURCESET', Blockly.lua.ORDER_NONE) || 'nil';
	return [object + ':at(Ressource.' + this.getTitleValue('RESSOURCE') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};


//RessourceSet contain
Blockly.Language.dronewars_ressourceSet_contains =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET,
init:
    function()
    {
      this.setColour(230);
      this.setInputsInline(true);
      this.appendValueInput('OUT')
      .setCheck('RessourceSet')
      .appendTitle(Blockly.LANG_DRONEWARS_CONTAINS_1);
      this.appendValueInput('IN')
      .setCheck('RessourceSet')
      .appendTitle(Blockly.LANG_DRONEWARS_CONTAINS_2);
        this.setOutput(true, Boolean);
    }
};

Blockly.lua.dronewars_ressourceSet_contains = function()
{
    var out = Blockly.lua.valueToCode(this, 'OUT', Blockly.lua.ORDER_NONE) || 'RessourceSet()';
    var inner = Blockly.lua.valueToCode(this, 'IN', Blockly.lua.ORDER_NONE) || 'RessourceSet()';

    return [out + ':contains(' + inner + ')',
            Blockly.lua.ORDER_FUNCTION_CALL
           ];
};
