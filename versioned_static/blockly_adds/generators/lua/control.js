/**
 * Visual Blocks Language
 *
 * Copyright 2012 Google Inc.
 * http://code.google.com/p/blockly/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview Generating Python for control blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

Blockly.lua = Blockly.Generator.get('lua');

Blockly.lua.controls_if = function()
{
	// If/elseif/else condition.
	var n = 0;
	var argument = Blockly.lua.valueToCode(this, 'IF' + n,
	                                       Blockly.lua.ORDER_NONE) || 'false';
	var branch = Blockly.lua.statementToCode(this, 'DO' + n) || '';
	var code = 'if ' + argument + ' then\n' + branch;
	for(n = 1; n <= this.elseifCount_; n++)
	{
		argument = Blockly.lua.valueToCode(this, 'IF' + n,
		                                   Blockly.lua.ORDER_NONE) || 'false';
		branch = Blockly.lua.statementToCode(this, 'DO' + n) || '\n';
		code += 'elseif ' + argument + ' then\n' + branch;
	}
	if(this.elseCount_)
	{
		branch = Blockly.lua.statementToCode(this, 'ELSE') || '\n';
		code += 'else\n' + branch;
	}
	return code + 'end\n';
};

Blockly.lua.controls_whileUntil = function()
{
	// Do while/until loop.
	var until = this.getTitleValue('MODE') == 'UNTIL';
	var argument0 = Blockly.lua.valueToCode(this, 'BOOL',
	                                        until ? Blockly.lua.ORDER_LOGICAL_NOT :
	                                        Blockly.lua.ORDER_NONE) || 'false';
	var branch0 = Blockly.lua.statementToCode(this, 'DO') || '';
	if(this.getTitleValue('MODE') == 'UNTIL')
	{
		if(!argument0.match( / ^ \w + $ /))
		{
			argument0 = '(' + argument0 + ')';
		}
		argument0 = 'not(' + argument0 + ')';
	}
	return 'while ' + argument0 + ' do\n' + branch0 + '  ::continue::\nend\n';
};

Blockly.lua.controls_for = function()
{
	// For loop.
	var variable0 = Blockly.lua.variableDB_.getName(
	                  this.getInputVariable('VAR'), Blockly.Variables.NAME_TYPE);
	var argument0 = Blockly.lua.valueToCode(this, 'FROM',
	                                        Blockly.lua.ORDER_NONE) || '0';
	// If starting index is 0, omit it.
	argument0 = parseInt(argument0, 10);
	var argument1 = Blockly.lua.valueToCode(this, 'TO',
	                                        Blockly.lua.ORDER_ADDITIVE) || '0';
	if(argument1.match( / ^ \d + $ /))
	{
		// If the index is a naked number, increment it right now.
		argument1 = parseInt(argument1, 10);
	}
	else
	{
		// If the index is dynamic, increment it in code.
		//argument1 += ' + 1';
	}
	var branch0 = Blockly.lua.statementToCode(this, 'DO') || '  pass\n';
	var code = 'for ' + variable0 + ' = ' + argument0 + ',' + argument1 + ',1 do\n' + branch0 + '  ::continue::\nend\n';
	return code;
};

Blockly.lua.controls_forEach = function()
{
	// For each loop.
	var variable0 = Blockly.lua.variableDB_.getName(
	                  this.getInputVariable('VAR'), Blockly.Variables.NAME_TYPE);
	var argument0 = Blockly.lua.valueToCode(this, 'LIST',
	                                        Blockly.lua.ORDER_RELATIONAL) || '{}';
	var branch0 = Blockly.lua.statementToCode(this, 'DO') || '';

	//var getRangeFunc = 'function(cont) if type(cont) == \'table\' then return ipairs(cont) else return cont:range() end end'
	//var code = 'for _,' + variable0 + ' in (' + getRangeFunc + ')(' + argument0 + ') do\n' + branch0 + '  ::continue::\nend\n';
	var code = 'for _,' + variable0 + ' in ipairs(' + argument0 + ') do\n' + branch0 + '  ::continue::\nend\n';
	return code;
};

Blockly.lua.controls_flow_statements = function()
{
	// Flow statements: continue, break.
	switch(this.getTitleValue('FLOW'))
	{
	case 'BREAK':
		return 'break\n';
	case 'CONTINUE':
		return 'goto continue\n';
	}
	throw 'Unknown flow statement.';
};
