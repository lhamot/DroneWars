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
 * @fileoverview Generating Lua for list blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

'use strict';

goog.provide('Blockly.Lua.lists');

goog.require('Blockly.Lua');

Blockly.Lua['lists_create_empty'] = function(block)
{
	// Create an empty list.
	return ['{}', Blockly.Lua.ORDER_ATOMIC];
};

Blockly.Lua['lists_create_with'] = function(block)
{
	// Create a list with any number of elements of any type.
	var code = new Array(block.itemCount_);
	for(var n = 0; n < block.itemCount_; n++)
	{
		code[n] = Blockly.Lua.valueToCode(block, 'ADD' + n,
		                                  Blockly.Lua.ORDER_NONE) || 'None';
	}
	code = '{' + code.join(', ') + '}';
	return [code, Blockly.Lua.ORDER_ATOMIC];
};

Blockly.Lua['lists_repeat'] = function(block)
{
	// Create a list with one element repeated.
	var value = Blockly.Lua.valueToCode(block, 'ITEM',
	                                    Blockly.Lua.ORDER_NONE) || 'None';
	var count = Blockly.Lua.valueToCode(block, 'NUM',
	                                    Blockly.Lua.ORDER_MULTIPLICATIVE) || '0';
	//var code = '{' + argument0 + '} * ' + argument1;
	var code =
	  '(' +
	  'function() ' +
	  'result = {} ' +
	  'for i=1,' + count + ',1 do result[i] = ' + value + ' end ' +
	  'return result ' +
	  'end' +
	  ')()';
	return [code, Blockly.Lua.ORDER_MULTIPLICATIVE];
};

Blockly.Lua['lists_length'] = function(block)
{
	// Testing the length of a list is the same as for a string.
	return Blockly.Lua.text_length.call(block);
};

Blockly.Lua['lists_isEmpty'] = function(block)
{
	// Testing a list for being empty is the same as for a string.
	return Blockly.Lua.text_isEmpty.call(block);
};

Blockly.Lua['lists_indexOf'] = function(block)
{
	// Search the text for a substring.
	// Should we allow for non-case sensitive???
	var value = Blockly.Lua.valueToCode(block, 'FIND',
	                                    Blockly.Lua.ORDER_NONE) || '\'\'';
	var tableName = Blockly.Lua.valueToCode(block, 'VALUE',
	                                        Blockly.Lua.ORDER_MEMBER) || '\'\'';
	var code = '';
	if(block.getTitleValue('END') == 'FIRST')
	{
		code =
		  '(function() for k,v in ipairs(' + tableName + ') do if v == ' + value + ' then return k end end end)()';
	}
	else
	{
		code =
		  '(function() for i = #' + tableName + ',1,-1 do if ' + tableName + '[i] == ' + value + ' then return i end end end)()';
	}
	//var operator = block.getTitleValue('END') == 'FIRST' ? 'find' : 'rfind';
	//var code = argument1 + '.' + operator + '(' + argument0 + ') + 1';
	return [code, Blockly.Lua.ORDER_MEMBER];
}

Blockly.Lua['lists_getIndex'] = function(block)
{
	// Indexing into a list is the same as indexing into a string.
	return Blockly.Lua.text_charAt.call(block);
};

Blockly.Lua['lists_setIndex'] = function(block)
{
	// Set element at index.
	var argument0 = Blockly.Lua.valueToCode(block, 'AT',
	                                        Blockly.Lua.ORDER_NONE) || '1';
	var argument1 = Blockly.Lua.valueToCode(block, 'LIST',
	                                        Blockly.Lua.ORDER_MEMBER) || '{}';
	var argument2 = Blockly.Lua.valueToCode(block, 'TO',
	                                        Blockly.Lua.ORDER_NONE) || 'nil';
	// Blockly uses one-based indicies.
	if(argument0.match( / ^ \d + $ /))
	{
		// If the index is a naked number, decrement it right now.
		// Except we don't allow negative index like in Lua.
		argument0 = Math.max(0, parseInt(argument0, 10));
	}
	else
	{
		// If the index is dynamic, decrement it in code.
		//argument0 += ' - 1';
	}
	var code = argument1 + '[' + argument0 + '] = ' + argument2 + '\n';
	return code;
};
