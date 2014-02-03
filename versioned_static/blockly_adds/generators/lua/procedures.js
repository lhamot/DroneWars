/**
 * Visual Blocks Language
 *
 * Copyright 2012 Google Inc.
 * http://blockly.googlecode.com/
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
 * @fileoverview Generating Lua for procedures blocks.
 * @author Loïc HAMOT
 */
'use strict';

goog.provide('Blockly.Lua.procedures');

goog.require('Blockly.Lua');


Blockly.Lua['procedures_defreturn'] = function(block) {
  // Define a procedure with a return value.
	// First, add a 'global' statement for every variable that is assigned.
	/*var globals = Blockly.Variables.allVariables(block);
	for (var i = globals.length - 1; i >= 0; i--) {
	  var varName = globals[i];
	  if (block.arguments_.indexOf(varName) == -1) {
	      globals[i] = Blockly.Lua.variableDB_.getName(varName,
	        Blockly.Variables.NAME_TYPE);
	  } else {
	    // block variable has been explicitly passed to the function.
	    // Do not include it in the list of globals.
	    globals.splice(i, 1);
	  }
	}
	globals = globals.length ? '  global ' + globals.join(', ') + '\n' : '';  */
	var funcName = block.getFieldValue('NAME');
	if(funcName.indexOf("AI:") != 0)
		funcName = Blockly.Lua.variableDB_.getName(funcName,
		           Blockly.Procedures.NAME_TYPE);
	var branch = Blockly.Lua.statementToCode(block, 'STACK');
	var returnValue = Blockly.Lua.valueToCode(block, 'RETURN',
	                  Blockly.Lua.ORDER_NONE) || '';
	if(returnValue)
	{
		returnValue = '  return ' + returnValue + '\n';
	}
	else if(!branch)
	{
		//branch = '  return\n';
	}
	var args = [];
	for(var x = 0; x < block.arguments_.length; x++)
	{
		args[x] = Blockly.Lua.variableDB_.getName(block.arguments_[x],
		          Blockly.Variables.NAME_TYPE);
	}
	var code = 'function ' + funcName + '(' + args.join(', ') + ')\n' +
	           branch + returnValue + 'end';
	code = Blockly.Lua.scrub_(block, code);
	Blockly.Lua.definitions_[funcName] = code;
  return null;
};

// Defining a procedure without a return value uses the same generator as
// a procedure with a return value.
Blockly.Lua['procedures_defnoreturn'] =
    Blockly.Lua['procedures_defreturn'];

Blockly.Lua['procedures_callreturn'] = function(block) {
  // Call a procedure with a return value.
	var funcName = Blockly.Lua.variableDB_.getName(block.getFieldValue('NAME'),
	               Blockly.Procedures.NAME_TYPE);
	var args = [];
	for(var x = 0; x < block.arguments_.length; x++)
	{
		args[x] = Blockly.Lua.valueToCode(block, 'ARG' + x,
		                                  Blockly.Lua.ORDER_NONE) || 'nil';
	}
	var code = funcName + '(' + args.join(', ') + ')';
	return [code, Blockly.Lua.ORDER_FUNCTION_CALL];
};

Blockly.Lua['procedures_callnoreturn'] = function(block) {
  // Call a procedure with no return value.
	var funcName = Blockly.Lua.variableDB_.getName(block.getFieldValue('NAME'),
	               Blockly.Procedures.NAME_TYPE);
	var args = [];
	for(var x = 0; x < block.arguments_.length; x++)
	{
		args[x] = Blockly.Lua.valueToCode(block, 'ARG' + x,
		                                  Blockly.Lua.ORDER_NONE) || 'nil';
	}
	var code = funcName + '(' + args.join(', ') + ')\n';
  return code;
};


Blockly.Lua['procedures_ifreturn'] = function(block) {
  // Conditionally return value from a procedure.
    var condition = Blockly.Lua.valueToCode(block, 'CONDITION',
        Blockly.Lua.ORDER_NONE) || 'False';
    var code = 'if(' + condition + ') then\n';
    if (block.hasReturnValue_) {
        var value = Blockly.Lua.valueToCode(block, 'VALUE',
            Blockly.Lua.ORDER_NONE) || 'None';
        code += '  return ' + value + '\nend\n';
    } else {
        code += '  return\nend\n';
    }
    return code;
};

Blockly.Lua['procedures_return'] = function () {
    var code = '';
    if (block.hasReturnValue_) {
        var value = Blockly.Lua.valueToCode(block, 'VALUE',
            Blockly.Lua.ORDER_NONE) || 'None';
        code += 'return ' + value + '\n';
    } else {
        code += 'return\n';
    }
    return code;
};
