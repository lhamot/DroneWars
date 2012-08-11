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

Blockly.lua.variables_get = function()
{
	// Variable getter.
	code = Blockly.lua.variableDB_.getName(this.getTitleText('VAR'),
	                                       Blockly.Variables.NAME_TYPE);
	return [code, Blockly.lua.ORDER_ATOMIC];
};

Blockly.lua.variables_set = function()
{
	// Variable setter.
	var argument0 = Blockly.lua.valueToCode(this, 'VALUE',
	                                        Blockly.lua.ORDER_NONE) || 'nil';
	var varName = Blockly.lua.variableDB_.getName(this.getTitleText('VAR'),
	              Blockly.Variables.NAME_TYPE);
	return varName + ' = ' + argument0 + '\n';
};
