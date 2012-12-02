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
 * @fileoverview Generating lua for list blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

Blockly.lua = Blockly.Generator.get('lua');

Blockly.lua.lists_create_empty = function()
{
	// Create an empty list.
	return ['{}', Blockly.lua.ORDER_ATOMIC];
};

Blockly.lua.lists_create_with = function()
{
	// Create a list with any number of elements of any type.
	var code = new Array(this.itemCount_);
	for(var n = 0; n < this.itemCount_; n++)
	{
		code[n] = Blockly.lua.valueToCode(this, 'ADD' + n,
		                                  Blockly.lua.ORDER_NONE) || 'None';
	}
	code = '{' + code.join(', ') + '}';
	return [code, Blockly.lua.ORDER_ATOMIC];
};

Blockly.lua.lists_repeat = function()
{
	// Create a list with one element repeated.
	var value = Blockly.lua.valueToCode(this, 'ITEM',
	                                    Blockly.lua.ORDER_NONE) || 'None';
	var count = Blockly.lua.valueToCode(this, 'NUM',
	                                    Blockly.lua.ORDER_MULTIPLICATIVE) || '0';
	//var code = '{' + argument0 + '} * ' + argument1;
	var code =
	  '(' +
	  'function() ' +
	  'result = {} ' +
	  'for i=1,' + count + ',1 do result[i] = ' + value + ' end ' +
	  'return result ' +
	  'end' +
	  ')()';
	return [code, Blockly.lua.ORDER_MULTIPLICATIVE];
};

Blockly.lua.lists_length = function()
{
	// Testing the length of a list is the same as for a string.
	return Blockly.lua.text_length.call(this);
};

Blockly.lua.lists_isEmpty = function()
{
	// Testing a list for being empty is the same as for a string.
	return Blockly.lua.text_isEmpty.call(this);
};

Blockly.lua.lists_indexOf = function()
{
	// Search the text for a substring.
	// Should we allow for non-case sensitive???
	var value = Blockly.lua.valueToCode(this, 'FIND',
	                                    Blockly.lua.ORDER_NONE) || '\'\'';
	var tableName = Blockly.lua.valueToCode(this, 'VALUE',
	                                        Blockly.lua.ORDER_MEMBER) || '\'\'';
	var code = '';
	if(this.getTitleValue('END') == 'FIRST')
	{
		code =
		  '(function() for k,v in ipairs(' + tableName + ') do if v == ' + value + ' then return k end end end)()';
	}
	else
	{
		code =
		  '(function() for i = #' + tableName + ',1,-1 do if ' + tableName + '[i] == ' + value + ' then return i end end end)()';
	}
	//var operator = this.getTitleValue('END') == 'FIRST' ? 'find' : 'rfind';
	//var code = argument1 + '.' + operator + '(' + argument0 + ') + 1';
	return [code, Blockly.lua.ORDER_MEMBER];
}

Blockly.lua.lists_getIndex = function()
{
	// Indexing into a list is the same as indexing into a string.
	return Blockly.lua.text_charAt.call(this);
};

Blockly.lua.lists_setIndex = function()
{
	// Set element at index.
	var argument0 = Blockly.lua.valueToCode(this, 'AT',
	                                        Blockly.lua.ORDER_NONE) || '1';
	var argument1 = Blockly.lua.valueToCode(this, 'LIST',
	                                        Blockly.lua.ORDER_MEMBER) || '{}';
	var argument2 = Blockly.lua.valueToCode(this, 'TO',
	                                        Blockly.lua.ORDER_NONE) || 'nil';
	// Blockly uses one-based indicies.
	if(argument0.match( / ^ \d + $ /))
	{
		// If the index is a naked number, decrement it right now.
		// Except we don't allow negative index like in lua.
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
