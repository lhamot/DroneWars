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
 * @fileoverview Generating lua for variable blocks.
 * @author Loïc HAMOT
 */

Blockly.lua = Blockly.Generator.get('lua');

Blockly.lua.procedures_defreturn = function()
{
	// Define a procedure with a return value.
	// First, add a 'global' statement for every variable that is assigned.
	/*var globals = Blockly.Variables.allVariables(this);
	for (var i = globals.length - 1; i >= 0; i--) {
	  var varName = globals[i];
	  if (this.arguments_.indexOf(varName) == -1) {
	      globals[i] = Blockly.lua.variableDB_.getName(varName,
	        Blockly.Variables.NAME_TYPE);
	  } else {
	    // This variable has been explicitly passed to the function.
	    // Do not include it in the list of globals.
	    globals.splice(i, 1);
	  }
	}
	globals = globals.length ? '  global ' + globals.join(', ') + '\n' : '';  */
	var funcName = this.getTitleValue('NAME');
	if(funcName.indexOf("AI:") != 0)
		funcName = Blockly.lua.variableDB_.getName(funcName,
		           Blockly.Procedures.NAME_TYPE);
	var branch = Blockly.lua.statementToCode(this, 'STACK');
	var returnValue = Blockly.lua.valueToCode(this, 'RETURN',
	                  Blockly.lua.ORDER_NONE) || '';
	if(returnValue)
	{
		returnValue = '  return ' + returnValue + '\n';
	}
	else if(!branch)
	{
		//branch = '  return\n';
	}
	var args = [];
	for(var x = 0; x < this.arguments_.length; x++)
	{
		args[x] = Blockly.lua.variableDB_.getName(this.arguments_[x],
		          Blockly.Variables.NAME_TYPE);
	}
	var code = 'function ' + funcName + '(' + args.join(', ') + ')\n' +
	           branch + returnValue + 'end';
	code = Blockly.lua.scrub_(this, code);
	Blockly.lua.definitions_[funcName] = code;
	return null;
};

// Defining a procedure without a return value uses the same generator as
// a procedure with a return value.
Blockly.lua.procedures_defnoreturn =
  Blockly.lua.procedures_defreturn;

Blockly.lua.procedures_callreturn = function()
{
	// Call a procedure with a return value.
	var funcName = Blockly.lua.variableDB_.getName(this.getTitleValue('NAME'),
	               Blockly.Procedures.NAME_TYPE);
	var args = [];
	for(var x = 0; x < this.arguments_.length; x++)
	{
		args[x] = Blockly.lua.valueToCode(this, 'ARG' + x,
		                                  Blockly.lua.ORDER_NONE) || 'nil';
	}
	var code = funcName + '(' + args.join(', ') + ')';
	return [code, Blockly.lua.ORDER_FUNCTION_CALL];
};

Blockly.lua.procedures_callnoreturn = function()
{
	// Call a procedure with no return value.
	var funcName = Blockly.lua.variableDB_.getName(this.getTitleValue('NAME'),
	               Blockly.Procedures.NAME_TYPE);
	var args = [];
	for(var x = 0; x < this.arguments_.length; x++)
	{
		args[x] = Blockly.lua.valueToCode(this, 'ARG' + x,
		                                  Blockly.lua.ORDER_NONE) || 'nil';
	}
	var code = funcName + '(' + args.join(', ') + ')\n';
	return code;
};


Blockly.lua.procedures_ifreturn = function () {
    // Conditionally return value from a procedure.
    var condition = Blockly.lua.valueToCode(this, 'CONDITION',
        Blockly.lua.ORDER_NONE) || 'False';
    var code = 'if(' + condition + ') then\n';
    if (this.hasReturnValue_) {
        var value = Blockly.lua.valueToCode(this, 'VALUE',
            Blockly.lua.ORDER_NONE) || 'None';
        code += '  return ' + value + '\nend\n';
    } else {
        code += '  return\nend\n';
    }
    return code;
};

Blockly.lua.procedures_return = function () {
    var code = '';
    if (this.hasReturnValue_) {
        var value = Blockly.lua.valueToCode(this, 'VALUE',
            Blockly.lua.ORDER_NONE) || 'None';
        code += 'return ' + value + '\n';
    } else {
        code += 'return\n';
    }
    return code;
};