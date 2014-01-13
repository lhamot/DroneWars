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
 * @fileoverview Generating Lua for text blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

'use strict';

goog.provide('Blockly.Lua.text');

goog.require('Blockly.Lua');

Blockly.Lua['text'] = function(block)
{
	// Text value.
	var code = Blockly.Lua.quote_(block.getTitleValue('TEXT'));
	return [code, Blockly.Lua.ORDER_ATOMIC];
};

Blockly.Lua['text_join'] = function(block)
{
	// Create a string made up of any number of elements of any type.
	//Should we allow joining by '-' or ',' or any other characters?
	var code = '';
	for(var n = 0; n < block.itemCount_; n++)
	{
		var token = Blockly.Lua.valueToCode(
		              block, 'ADD' + n, Blockly.Lua.ORDER_NONE) || '\'\''
		            if(n != 0)
			            code += ' .. '
			                    code += 'tostring(' + token + ')';
	}
	if(block.itemCount_ > 1)
		code = '(' + code + ')';
	return [code, Blockly.Lua.ORDER_FUNCTION_CALL];
};

Blockly.Lua['text_length'] = function(block)
{
	// String length.
	var argument0 = Blockly.Lua.valueToCode(block, 'VALUE',
	                                        Blockly.Lua.ORDER_NONE) || '\'\'';
	return ['(#' + argument0 + ')', Blockly.Lua.ORDER_FUNCTION_CALL];
};

Blockly.Lua['text_isEmpty'] = function(block)
{
	// Is the string null?
	var argument0 = Blockly.Lua.valueToCode(block, 'VALUE',
	                                        Blockly.Lua.ORDER_NONE) || '\'\'';
	var code = '((#' + argument0 + ') == 0)';
	return [code, Blockly.Lua.ORDER_LOGICAL_NOT];
};

Blockly.Lua['text_endString'] = function(block)
{
	// Return a leading or trailing substring.
	// Do we need to prevent 'List index out of range' ERROR by checking
	// if argument 0 > len(argument1)? Or will ALL error be handled systematically?
	var first = block.getTitleValue('END') == 'FIRST';
	var argument0 = Blockly.Lua.valueToCode(block, 'NUM',
	                                        Blockly.Lua.ORDER_NONE) || '1';
	var argument1 = Blockly.Lua.valueToCode(block, 'TEXT',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	var code;
	if(first)
		code = 'string.sub(' + argument1 + ',1,' + argument0 + ')';
	else
		code = 'string.sub(' + argument1 + ',-' + argument0 + ')';
	return [code, Blockly.Lua.ORDER_MEMBER];
};

Blockly.Lua['text_indexOf'] = function(block)
{
	// Search the text for a substring.
	// Should we allow for non-case sensitive???
	var first = block.getTitleValue('END') == 'FIRST';
	var argument0 = Blockly.Lua.valueToCode(block, 'FIND',
	                                        Blockly.Lua.ORDER_NONE) || '\'\'';
	var argument1 = Blockly.Lua.valueToCode(block, 'VALUE',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	//var code = argument1 + '.' + operator + '(' + argument0 + ') + 1';
	if(first)
		var code = '(function() b,e = string.find(' + argument1 + ',' + argument0 + ') return b end)()';
	else
		var code = '(function() b,e = string.find(' + argument1 + ',\'.*\'..' + argument0 + ') ' +
		           'if e == nil then return nil end ' +
		           'return ((e + 1) - #' + argument0 + ') ' +
		           'end)()';
	return [code, Blockly.Lua.ORDER_MEMBER];
};

Blockly.Lua['text_charAt'] = function(block)
{
	// Get letter at index.
	var argument0 = Blockly.Lua.valueToCode(block, 'AT',
	                                        Blockly.Lua.ORDER_NONE) || '1';
	var argument1 = Blockly.Lua.valueToCode(block, 'VALUE',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	// Blockly uses one-based indicies.
	if(argument0.match( / ^ \d + $ /))
	{
		// If the index is a naked number, decrement it right now.
		// Except not allowing negative index by constraining at 0.
		argument0 = Math.max(0, parseInt(argument0, 10));
	}
	else
	{
		// If the index is dynamic, decrement it in code.
		//argument0 += ' - 1';
	}
	var code = '(' + argument1 + '):sub(' + argument0 + ',' + argument0 + ')';
	return [code, Blockly.Lua.ORDER_MEMBER];
};

Blockly.Lua['text_changeCase'] = function(block)
{
	// Change capitalization.
	var argument0 = Blockly.Lua.valueToCode(block, 'TEXT',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	var mode = block.getTitleValue('CASE');
	if(mode == 'TITLECASE')
	{
		var code =
		  '(function(str) ' +
		  'local buf = {} ' +
		  'for word in string.gmatch(str, "%S+") do ' +
		  'local first, rest = string.sub(word, 1, 1), string.sub(word, 2) ' +
		  'table.insert(buf, string.upper(first) .. string.lower(rest)) ' +
		  'end ' +
		  'return table.concat(buf, " ") ' +
		  'end)(' + argument0 + ')';
		return [code, Blockly.Lua.ORDER_MEMBER];
	}
	else
	{
		var operator = Blockly.Lua.text_changeCase.OPERATORS[mode];
		var code = '(' + argument0 + ')' + operator;
		return [code, Blockly.Lua.ORDER_MEMBER];
	}
};

Blockly.Lua['text_changeCase'].OPERATORS =
{
UPPERCASE: ':upper()'
	,
LOWERCASE: ':lower()'
};

Blockly.Lua['text_trim'] = function(block)
{
	// Trim spaces.
	var mode = block.getTitleValue('MODE');
	var lambda = Blockly.Lua.text_trim.OPERATORS[mode];
	var argument0 = Blockly.Lua.valueToCode(block, 'TEXT',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	var code = '(' + lambda + ')(' + argument0 + ')';
	return [code, Blockly.Lua.ORDER_MEMBER];
};

Blockly.Lua['text_trim'].OPERATORS =
{
LEFT: 'function(s) return (s:gsub("^%s*", "")) end'
	,
RIGHT: 'function(s) local n = #s while n > 0 and s:find("^%s", n) do n = n - 1 end return s:sub(1, n) end'
	,
BOTH: 'function(s) return (s:gsub("^%s*(.-)%s*$", "%1")) end'
};

Blockly.Lua['text_print'] = function(block)
{
	// Print statement.
	var argument0 = Blockly.Lua.valueToCode(block, 'TEXT',
	                                        Blockly.Lua.ORDER_NONE) || '\'\'';
	return 'print(' + argument0 + ')\n';
};

Blockly.Lua['text_prompt'] = function(block)
{
	// Prompt function.
	var msg = Blockly.Lua.quote_(block.getTitleValue('TEXT'));
	//var code = 'raw_input(' + msg + ')';
	var code = 'function(msg) print(msg) value = io.stdin:read\'*l\' return value end';
	code = '(' + code + ')(' + msg + ')';
	var toNumber = block.getTitleValue('TYPE') == 'NUMBER';
	if(toNumber)
	{
		code = 'tonumber(' + code + ')';
	}
	return [code, Blockly.Lua.ORDER_FUNCTION_CALL];
};
