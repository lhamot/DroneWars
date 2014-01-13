/**
 * Visual Blocks Language
 *
 * Copyright 2012 Google Inc.
 * http://code.google.com/p/blockly/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use block file except in compliance with the License.
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
 * @fileoverview Generating LUA for control blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

'use strict';

goog.provide('Blockly.Lua.controls');

goog.require('Blockly.Lua');

Blockly.Lua['controls_if'] = function(block)
{
	// If/elseif/else condition.
	var n = 0;
	var argument = Blockly.Lua.valueToCode(block, 'IF' + n,
	                                       Blockly.Lua.ORDER_NONE) || 'false';
	var branch = Blockly.Lua.statementToCode(block, 'DO' + n) || '';
	var code = 'if ' + argument + ' then\n' + branch;
	for(n = 1; n <= block.elseifCount_; n++)
	{
		argument = Blockly.Lua.valueToCode(block, 'IF' + n,
		                                   Blockly.Lua.ORDER_NONE) || 'false';
		branch = Blockly.Lua.statementToCode(block, 'DO' + n) || '\n';
		code += 'elseif ' + argument + ' then\n' + branch;
	}
	if(block.elseCount_)
	{
		branch = Blockly.Lua.statementToCode(block, 'ELSE') || '\n';
		code += 'else\n' + branch;
	}
	return code + 'end\n';
};

Blockly.Lua['controls_whileUntil'] = function(block)
{
	// Do while/until loop.
	var until = block.getTitleValue('MODE') == 'UNTIL';
	var argument0 = Blockly.Lua.valueToCode(block, 'BOOL',
	                                        until ? Blockly.Lua.ORDER_LOGICAL_NOT :
	                                        Blockly.Lua.ORDER_NONE) || 'false';
	var branch0 = Blockly.Lua.statementToCode(block, 'DO') || '';
	if(block.getTitleValue('MODE') == 'UNTIL')
	{
		if(!argument0.match( / ^ \w + $ /))
		{
			argument0 = '(' + argument0 + ')';
		}
		argument0 = 'not(' + argument0 + ')';
	}
	return 'while ' + argument0 + ' do\n' + branch0 + '  ::continue::\nend\n';
};

Blockly.Lua['controls_for'] = function(block)
{
	// For loop.
	var variable0 = Blockly.Lua.variableDB_.getName(
	                  block.getTitleValue('VAR'), Blockly.Variables.NAME_TYPE);
	var argument0 = Blockly.Lua.valueToCode(block, 'FROM',
	                                        Blockly.Lua.ORDER_NONE) || '0';
	// If starting index is 0, omit it.
	argument0 = parseInt(argument0, 10);
	var argument1 = Blockly.Lua.valueToCode(block, 'TO',
	                                        Blockly.Lua.ORDER_ADDITIVE) || '0';
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
	var branch0 = Blockly.Lua.statementToCode(block, 'DO') || '  pass\n';
	var code = 'for ' + variable0 + ' = ' + argument0 + ',' + argument1 + ',1 do\n' + branch0 + '  ::continue::\nend\n';
	return code;
};

Blockly.Lua['controls_forEach'] = function(block)
{
	// For each loop.
    var variable0 = Blockly.Lua.variableDB_.getName(
                      block.getTitleValue('VAR'), Blockly.Variables.NAME_TYPE);	                  
	var argument0 = Blockly.Lua.valueToCode(block, 'LIST',
	                                        Blockly.Lua.ORDER_RELATIONAL) || '{}';
	var branch0 = Blockly.Lua.statementToCode(block, 'DO') || '';

	//var getRangeFunc = 'function(cont) if type(cont) == \'table\' then return ipairs(cont) else return cont:range() end end'
	//var code = 'for _,' + variable0 + ' in (' + getRangeFunc + ')(' + argument0 + ') do\n' + branch0 + '  ::continue::\nend\n';
	var code = 'for _,' + variable0 + ' in ipairs(' + argument0 + ') do\n' + branch0 + '  ::continue::\nend\n';
	return code;
};

Blockly.Lua['controls_flow_statements'] = function(block)
{
	// Flow statements: continue, break.
	switch(block.getTitleValue('FLOW'))
	{
	case 'BREAK':
		return 'break\n';
	case 'CONTINUE':
		return 'goto continue\n';
	}
	throw 'Unknown flow statement.';
};
