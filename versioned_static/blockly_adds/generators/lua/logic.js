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
 * @fileoverview Generating Lua for logic blocks.
 * @author Loïc HAMOT
 */

'use strict';

goog.provide('Blockly.Lua.logic');

goog.require('Blockly.Lua');

Blockly.Lua['logic_compare'] = function(block)
{
	// Comparison operator.
	var mode = block.getFieldValue('OP');
	var operator = Blockly.Lua.logic_compare.OPERATORS[mode];
	var order = Blockly.Lua.ORDER_RELATIONAL;
	var argument0 = Blockly.Lua.valueToCode(block, 'A', order) || '0';
	var argument1 = Blockly.Lua.valueToCode(block, 'B', order) || '0';
	var code = argument0 + ' ' + operator + ' ' + argument1;
	return [code, order];
};

Blockly.Lua['logic_compare'].OPERATORS =
{
EQ: '=='
	,
NEQ: '~='
	,
LT: '<'
	,
LTE: '<='
	,
GT: '>'
	,
GTE: '>='
};

Blockly.Lua['logic_operation'] = function(block)
{
	// Operations 'and', 'or'.
	var operator = (block.getFieldValue('OP') == 'AND') ? 'and' : 'or';
	var order = (operator == 'and') ? Blockly.Lua.ORDER_LOGICAL_AND :
	            Blockly.Lua.ORDER_LOGICAL_OR;
	var argument0 = Blockly.Lua.valueToCode(block, 'A', order) || 'False';
	var argument1 = Blockly.Lua.valueToCode(block, 'B', order) || 'False';
	var code = argument0 + ' ' + operator + ' ' + argument1;
	return [code, order];
};

Blockly.Lua['logic_negate'] = function(block)
{
	// Negation.
	var argument0 = Blockly.Lua.valueToCode(block, 'BOOL',
	                                        Blockly.Lua.ORDER_LOGICAL_NOT) || 'false';
	var code = 'not (' + argument0 + ')';
	return [code, Blockly.Lua.ORDER_LOGICAL_NOT];
};

Blockly.Lua['logic_boolean'] = function(block)
{
	// Boolean values true and false.
	var code = (block.getFieldValue('BOOL') == 'TRUE') ? 'true' : 'false';
	return [code, Blockly.Lua.ORDER_ATOMIC];
};

Blockly.Lua['logic_null'] = function(block) {
  // Null data type.
  return ['nil', Blockly.Lua.ORDER_ATOMIC];
};

Blockly.Lua['logic_ternary'] = function(block) {
  // Ternary operator.
  var value_if = Blockly.Lua.valueToCode(block, 'IF',
      Blockly.Lua.ORDER_RELATIONAL) || 'false';
  var value_then = Blockly.Lua.valueToCode(block, 'THEN',
      Blockly.Lua.ORDER_RELATIONAL) || 'null';
  var value_else = Blockly.Lua.valueToCode(block, 'ELSE',
      Blockly.Lua.ORDER_RELATIONAL) || 'null';
  var code = value_if + ' and ' + value_then + ' or ' + value_else;
  return [code, Blockly.Lua.ORDER_RELATIONAL];
};
