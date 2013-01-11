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
 * @fileoverview Generating lua for math blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */
'use strict';
Blockly.lua = Blockly.Generator.get('lua');

Blockly.lua.math_number = function()
{
	// Numeric value.
  var code = window.parseFloat(this.getTitleValue('NUM'));
	return [code, Blockly.lua.ORDER_UNARY_SIGN];
};

Blockly.lua.math_arithmetic = function()
{
	// Basic arithmetic operators, and power.
	var mode = this.getTitleValue('OP');
	var tuple = Blockly.lua.math_arithmetic.OPERATORS[mode];
	var operator = tuple[0];
	var order = tuple[1];
	var argument0 = Blockly.lua.valueToCode(this, 'A', order) || '0';
	var argument1 = Blockly.lua.valueToCode(this, 'B', order) || '0';
	var code = argument0 + operator + argument1;
	return [code, order];
};

Blockly.lua.math_arithmetic.OPERATORS =
{
ADD:
	[' + ', Blockly.lua.ORDER_ADDITIVE],
MINUS:
	[' - ', Blockly.lua.ORDER_ADDITIVE],
MULTIPLY:
	[' * ', Blockly.lua.ORDER_MULTIPLICATIVE],
DIVIDE:
	[' / ', Blockly.lua.ORDER_MULTIPLICATIVE],
POWER:
	[' ^ ', Blockly.lua.ORDER_EXPONENTIATION]
};

Blockly.lua.math_change = function()
{
	// Add to a variable in place.
	var argument0 = Blockly.lua.valueToCode(this, 'DELTA',
	                                        Blockly.lua.ORDER_ADDITIVE) || '0';
	var varName = Blockly.lua.variableDB_.getName(this.getTitleValue('VAR'),
	              Blockly.Variables.NAME_TYPE);
	return varName + ' = (type(' + varName + ') == \'number\' and ' + varName + ' or 0) + 5';
};

Blockly.lua.math_single = function()
{
	var operator = this.getTitleValue('OP');
	// Math operators with single operand.
	if(operator == 'NEG')
	{
		var argNeg = Blockly.lua.valueToCode(this, 'NUM',
		                                     Blockly.lua.ORDER_UNARY_SIGN) || '0';
		return ['-' + argNeg, Blockly.lua.ORDER_UNARY_SIGN];
	}
	//Blockly.lua.definitions_['import_math'] = 'import math';
	var argNaked = Blockly.lua.valueToCode(this, 'NUM',
	                                       Blockly.lua.ORDER_NONE) || '0';
	var argParen = Blockly.lua.valueToCode(this, 'NUM',
	                                       Blockly.lua.ORDER_MULTIPLICATIVE) || '0';
	var code;
	// First, handle cases which generate values that don't need parentheses wrapping the code.
	switch(operator)
	{
	case 'ABS':
		code = 'math.abs(' + argNaked + ')';
		break;
	case 'ROOT':
		code = 'math.sqrt(' + argNaked + ')';
		break;
	case 'LN':
		code = 'math.log(' + argNaked + ')';
		break;
	case 'LOG10':
		code = 'math.log10(' + argNaked + ')';
		break;
	case 'EXP':
		code = 'math.exp(' + argNaked + ')';
		break;
	case 'POW10':
		code = 'math.pow(10,' + argNaked + ')';
		break;
	case 'ROUND':
		code = 'math.floor((' + argNaked + ') + 0.5)';
		break;
	case 'ROUNDUP':
		code = 'math.ceil(' + argNaked + ')';
		break;
	case 'ROUNDDOWN':
		code = 'math.floor(' + argNaked + ')';
		break;
	case 'SIN':
		code = 'math.sin(' + argParen + ' / 180 * math.pi)';
		break;
	case 'COS':
		code = 'math.cos(' + argParen + ' / 180 * math.pi)';
		break;
	case 'TAN':
		code = 'math.tan(' + argParen + ' / 180 * math.pi)';
		break;
	}
	if(code)
	{
		return [code, Blockly.lua.ORDER_FUNCTION_CALL];
	}
	// Second, handle cases which generate values that may need parentheses wrapping the code.
	switch(operator)
	{
	case 'ASIN':
		code = 'math.asin(' + argNaked + ') / math.pi * 180';
		break;
	case 'ACOS':
		code = 'math.acos(' + argNaked + ') / math.pi * 180';
		break;
	case 'ATAN':
		code = 'math.atan(' + argNaked + ') / math.pi * 180';
		break;
	default:
		throw 'Unknown math operator: ' + operator;
	}
	return [code, Blockly.lua.ORDER_MULTIPLICATIVE];
};

// Rounding functions have a single operand.
Blockly.lua.math_round = Blockly.lua.math_single;
// Trigonometry functions have a single operand.
Blockly.lua.math_trig = Blockly.lua.math_single;

Blockly.lua.math_on_list = function()
{
	// Math functions for lists.
	func = this.getTitleValue('OP');
	list = Blockly.lua.valueToCode(this, 'LIST',
	                               Blockly.lua.ORDER_NONE) || '[]';
	var code;
	switch(func)
	{
	case 'SUM':
	{
		var sumFunc = 'function(list) local result = 0 for _,k in ipairs(list) do result = result + k end return result end';
		code = '(' + sumFunc + ')(' + list + ')';
	}
	break;
	case 'MIN':
	{
		var minFunc = 'function(list) local result = math.huge for _,k in ipairs(list) do result = math.min(k, result) end return result end';
		code = '(' + minFunc + ')(' + list + ')';
	}
	break;
	case 'MAX':
	{
		var maxFunc = 'function(list) local result = -math.huge for _,k in ipairs(list) do result = math.max(k, result) end return result end';
		code = '(' + maxFunc + ')(' + list + ')';
	}
	break;
	case 'AVERAGE':
		if(!Blockly.lua.definitions_['math_mean'])
		{
			// This operation exclude null values: math_mean([null,null,1,9]) == 5.0.
			var functionName = Blockly.lua.variableDB_.getDistinctName(
			                     'math_mean', Blockly.Generator.NAME_TYPE);
			Blockly.lua.math_on_list.math_mean = functionName;
			var func = [];
			func.push('function ' + functionName + '(myList)');
			func.push('  local result = 0');
			func.push('  for _,k in ipairs(myList) do result = result + k end');
			func.push('  return result / (#myList)');
			func.push('end');
			Blockly.lua.definitions_['math_mean'] = func.join('\n');
		}
		code = Blockly.lua.math_on_list.math_mean + '(' + list + ')';
		break;
	case 'MEDIAN':
		if(!Blockly.lua.definitions_['math_median'])
		{
			// This operation exclude null values: math_median([null,null,1,3]) == 2.0.
			var functionName = Blockly.lua.variableDB_.getDistinctName('math_median',
			                   Blockly.Generator.NAME_TYPE);
			Blockly.lua.math_on_list.math_median = functionName;
			var func = [];
			func.push('function math_median(numlist)');
			func.push('  if type(numlist) ~= \'table\' then return numlist end');
			func.push('  table.sort(numlist)');
			func.push('  if #numlist %2 == 0 then return (numlist[#numlist/2] + numlist[#numlist/2+1]) / 2 end');
			func.push('  return numlist[math.ceil(#numlist/2)]');
			func.push('end');
			Blockly.lua.definitions_['math_median'] = func.join('\n');
		}
		code = Blockly.lua.math_on_list.math_median + '(' + list + ')';
		break;
	case 'MODE':
		if(!Blockly.lua.definitions_['math_modes'])
		{
			// As a list of numbers can contain more than one mode,
			// the returned result is provided as an array.
			// Mode of [3, 'x', 'x', 1, 1, 2, '3'] -> ['x', 1].
			var functionName = Blockly.lua.variableDB_.getDistinctName('math_modes',
			                   Blockly.Generator.NAME_TYPE);
			Blockly.lua.math_on_list.math_modes = functionName;
			var func = [];
			func.push('function math_modes(numlist)');
			func.push('  if type(numlist) ~= \'table\' then return numlist end');
			func.push('  local sets = {}');
			func.push('  local mode');
			func.push('  local modeValue = 0');
			//func.push('  table.foreach(numlist,function(i,v) if sets[v] then sets[v] = sets[v] + 1 else sets[v] = 1 end end)');
			func.push('  for _,v in ipairs(numlist) do if sets[v] then sets[v] = sets[v] + 1 else sets[v] = 1 end end');
			func.push('  for i,v in next,sets do');
			func.push('    if v > modeValue then');
			func.push('      modeValue = v');
			func.push('      mode = i');
			func.push('    else');
			func.push('      if v == modeValue then');
			func.push('        if type(mode) == \'table\' then');
			func.push('          table.insert(mode,i)');
			func.push('        else');
			func.push('          mode = {mode,i}');
			func.push('        end');
			func.push('      end');
			func.push('    end');
			func.push('  end');
			func.push('  return mode');
			func.push('end');
			Blockly.lua.definitions_['math_modes'] = func.join('\n');
		}
		code = Blockly.lua.math_on_list.math_modes + '(' + list + ')';
		break;
	case 'STD_DEV':
		if(!Blockly.lua.definitions_['math_standard_deviation'])
		{
			var functionName = Blockly.lua.variableDB_.getDistinctName(
			                     'math_standard_deviation', Blockly.Generator.NAME_TYPE);
			Blockly.lua.math_on_list.math_standard_deviation = functionName;
			var func = [];
			func.push('function math_standard_deviation( t )');
			func.push('  local sum_v2 = 0');
			func.push('  local sum_v = 0');
			func.push('  local count = 0');
			func.push('  for k,v in pairs(t) do');
			func.push('    if type(v) == \'number\' then');
			func.push('      sum_v = sum_v + v');
			func.push('      sum_v2 = sum_v2 + (v * v)');
			func.push('      count = count + 1');
			func.push('    end');
			func.push('  end');
			func.push('  if count == 0 then return nil end');
			func.push('  mean_v = sum_v / count');
			func.push('  result = math.sqrt((sum_v2 / count) - (mean_v * mean_v))');
			func.push('  return result');
			func.push('end');
			Blockly.lua.definitions_['math_standard_deviation'] = func.join('\n');
		}
		code = Blockly.lua.math_on_list.math_standard_deviation + '(' + list + ')';
		break;
	case 'RANDOM':
		//Blockly.lua.definitions_['import_random_choice'] = 'from random import choice';
		code = '(function(list) return list[math.random(#list)] end)(' + list + ')';
		break;
	default:
		throw 'Unknown operator.';
	}
	return [code, Blockly.lua.ORDER_FUNCTION_CALL];
};

Blockly.lua.math_constrain = function()
{
	// Constrain a number between two limits.
	var argument0 = Blockly.lua.valueToCode(this, 'VALUE',
	                                        Blockly.lua.ORDER_NONE) || '0';
	var argument1 = Blockly.lua.valueToCode(this, 'LOW',
	                                        Blockly.lua.ORDER_NONE) || '0';
	var argument2 = Blockly.lua.valueToCode(this, 'HIGH',
	                                        Blockly.lua.ORDER_NONE) || '0';
	code = 'math.min(math.max(' + argument0 + ', ' + argument1 + '), ' + argument2 + ')';
	return [code, Blockly.lua.ORDER_FUNCTION_CALL];
};

Blockly.lua.math_modulo = function()
{
	// Remainder computation.
	var argument0 = Blockly.lua.valueToCode(this, 'DIVIDEND',
	                                        Blockly.lua.ORDER_MULTIPLICATIVE) || '0';
	var argument1 = Blockly.lua.valueToCode(this, 'DIVISOR',
	                                        Blockly.lua.ORDER_MULTIPLICATIVE) || '0';
	var code = argument0 + ' % ' + argument1;
	return [code, Blockly.lua.ORDER_MULTIPLICATIVE];
};

Blockly.lua.math_random_int = function()
{
	// Random integer between [X] and [Y].
	//Blockly.lua.definitions_['import_random'] = 'import random';
	var argument0 = Blockly.lua.valueToCode(this, 'FROM',
	                                        Blockly.lua.ORDER_NONE) || '0';
	var argument1 = Blockly.lua.valueToCode(this, 'TO',
	                                        Blockly.lua.ORDER_NONE) || '0';
	code = 'math.random(' + argument0 + ', ' + argument1 + ')';
	return [code, Blockly.lua.ORDER_FUNCTION_CALL];
};

Blockly.lua.math_random_float = function()
{
	// Random fraction between 0 and 1.
	//Blockly.lua.definitions_['import_random'] = 'import random';
	return ['math.random()', Blockly.lua.ORDER_FUNCTION_CALL];
};
