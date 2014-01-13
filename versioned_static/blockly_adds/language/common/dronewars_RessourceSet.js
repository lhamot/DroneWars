'use strict';

goog.provide('Blockly.Lua');

//********************* RessourceSet ******************************************
Blockly.Blocks['dronewars_ressourceSet'] =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_RESSOURCESET,
init:
	function()
	{
		this.setColour(230);
		this.appendDummyInput()
      .appendField(Blockly.LANG_DRONEWARS_RESSOURCESET)
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'R1')
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'R2')
      .appendField(new Blockly.FieldTextInput('0', checkNumber), 'R3');
		this.setOutput(true, 'RessourceSet');
	}
};

Blockly.Lua['dronewars_ressourceSet'] = function(block)
{
	return ['RessourceSet(' + block.getTitleValue('R1') + ',' +
	        block.getTitleValue('R2') + ',' +
	        block.getTitleValue('R3') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};

Blockly.Blocks['dronewars_ressourceSet_at'] =
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
      .appendField(dropdown, 'RESSOURCE');
	  this.appendValueInput('RESSOURCESET')
      .setCheck('RessourceSet')
      .appendField(Blockly.LANG_DRONEWARS_RESSOURCESET_AT);
		this.setOutput(true, Number);
	}
};

Blockly.Lua['dronewars_ressourceSet_at'] = function(block)
{
	var object = Blockly.Lua.valueToCode(
	               block, 'RESSOURCESET', Blockly.Lua.ORDER_NONE) || 'nil';
	return [object + ':at(Ressource.' + block.getTitleValue('RESSOURCE') + ')',
	        Blockly.Lua.ORDER_HIGH
	       ];
};


//RessourceSet contain
Blockly.Blocks['dronewars_ressourceSet_contains'] =
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
      .appendField(Blockly.LANG_DRONEWARS_CONTAINS_1);
      this.appendValueInput('IN')
      .setCheck('RessourceSet')
      .appendField(Blockly.LANG_DRONEWARS_CONTAINS_2);
        this.setOutput(true, Boolean);
    }
};

Blockly.Lua['dronewars_ressourceSet_contains'] = function(block)
{
    var out = Blockly.Lua.valueToCode(block, 'OUT', Blockly.Lua.ORDER_NONE) || 'RessourceSet()';
    var inner = Blockly.Lua.valueToCode(block, 'IN', Blockly.Lua.ORDER_NONE) || 'RessourceSet()';

    return [out + ':contains(' + inner + ')',
            Blockly.Lua.ORDER_HIGH
           ];
};
