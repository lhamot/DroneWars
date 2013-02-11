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
 * @fileoverview Generating lua for logic blocks.
 * @author Loïc HAMOT
 */

Blockly.lua = Blockly.Generator.get('lua');

Blockly.lua.logic_compare = function()
{
	// Comparison operator.
	var mode = this.getTitleValue('OP');
	var operator = Blockly.lua.logic_compare.OPERATORS[mode];
	var order = Blockly.lua.ORDER_RELATIONAL;
	var argument0 = Blockly.lua.valueToCode(this, 'A', order) || '0';
	var argument1 = Blockly.lua.valueToCode(this, 'B', order) || '0';
	var code = argument0 + ' ' + operator + ' ' + argument1;
	return [code, order];
};

Blockly.lua.logic_compare.OPERATORS =
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

Blockly.lua.logic_operation = function()
{
	// Operations 'and', 'or'.
	var operator = (this.getTitleValue('OP') == 'AND') ? 'and' : 'or';
	var order = (operator == 'and') ? Blockly.lua.ORDER_LOGICAL_AND :
	            Blockly.lua.ORDER_LOGICAL_OR;
	var argument0 = Blockly.lua.valueToCode(this, 'A', order) || 'False';
	var argument1 = Blockly.lua.valueToCode(this, 'B', order) || 'False';
	var code = argument0 + ' ' + operator + ' ' + argument1;
	return [code, order];
};

Blockly.lua.logic_negate = function()
{
	// Negation.
	var argument0 = Blockly.lua.valueToCode(this, 'BOOL',
	                                        Blockly.lua.ORDER_LOGICAL_NOT) || 'false';
	var code = 'not (' + argument0 + ')';
	return [code, Blockly.lua.ORDER_LOGICAL_NOT];
};

Blockly.lua.logic_boolean = function()
{
	// Boolean values true and false.
	var code = (this.getTitleValue('BOOL') == 'TRUE') ? 'true' : 'false';
	return [code, Blockly.lua.ORDER_ATOMIC];
};

Blockly.lua.logic_null = function() {
  // Null data type.
  return ['nil', Blockly.lua.ORDER_ATOMIC];
};

Blockly.lua.logic_ternary = function() {
  // Ternary operator.
  var value_if = Blockly.lua.valueToCode(this, 'IF',
      Blockly.lua.ORDER_RELATIONAL) || 'false';
  var value_then = Blockly.lua.valueToCode(this, 'THEN',
      Blockly.lua.ORDER_RELATIONAL) || 'null';
  var value_else = Blockly.lua.valueToCode(this, 'ELSE',
      Blockly.lua.ORDER_RELATIONAL) || 'null';
  var code = value_if + ' and ' + value_then + ' or ' + value_else;
  return [code, Blockly.lua.ORDER_RELATIONAL];
};
