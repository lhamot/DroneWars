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
 * @fileoverview Helper functions for generating lua for blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */

Blockly.lua = Blockly.Generator.get('lua');

/**
 * List of illegal variable names.
 * This is not intended to be a security feature.  Blockly is 100% client-side,
 * so bypassing this list is trivial.  This is intended to prevent users from
 * accidentally clobbering a built-in object or function.
 * @private
 */
Blockly.lua.RESERVED_WORDS_ =
  // http://docs.python.org/reference/lexical_analysis.html#keywords
  'and,break,do,else,elseif,' +
  'end,false,for,function,if,' +
  'in,local,nil,not,or,' +
  'repeat,return,then,true,until,while';

/**
 * Order of operation ENUMs.
 * http://docs.python.org/reference/expressions.html#summary
 */
Blockly.lua.ORDER_LOGICAL_OR = 0;       // or
Blockly.lua.ORDER_LOGICAL_AND = 1;      // and
Blockly.lua.ORDER_RELATIONAL = 2;       // <     >     <=    >=    ~=    ==
Blockly.lua.ORDER_CONCATENATION = 3;    // ..
Blockly.lua.ORDER_UNARY_SIGN = 4;       // + -
Blockly.lua.ORDER_MULTIPLICATIVE = 5;   // * / %
Blockly.lua.ORDER_LOGICAL_NOT = 6;      // not # -
Blockly.lua.ORDER_EXPONENTIATION = 7;   // ^
Blockly.lua.ORDER_NONE = 99;            // (...)

/**
 * Initialise the database of variable names.
 */
Blockly.lua.init = function()
{
	// Create a dictionary of definitions to be printed before the code.
	Blockly.lua.definitions_ = {};

	if(Blockly.Variables)
	{
		if(!Blockly.lua.variableDB_)
		{
			Blockly.lua.variableDB_ =
			  new Blockly.Names(Blockly.lua.RESERVED_WORDS_.split(','));
		}
		else
		{
			Blockly.lua.variableDB_.reset();
		}

		var defvars = [];
		var variables = Blockly.Variables.allVariables();
		for(var x = 0; x < variables.length; x++)
		{
			defvars[x] = Blockly.lua.variableDB_.getDistinctName(variables[x],
			             Blockly.Variables.NAME_TYPE) + ' = nil';
		}
		Blockly.lua.definitions_['variables'] = defvars.join('\n');
	}
};

/**
 * Prepend the generated code with the variable definitions.
 * @param {string} code Generated code.
 * @return {string} Completed code.
 */
Blockly.lua.finish = function(code)
{
	// Convert the definitions dictionary into a list.
	var definitions = [];
	for(var name in Blockly.lua.definitions_)
	{
		definitions.push(Blockly.lua.definitions_[name]);
	}
	return definitions.join('\n') + '\n\n' + code;
};

/**
 * Naked values are top-level blocks with outputs that aren't plugged into
 * anything.
 * @param {string} line Line of generated code.
 * @return {string} Legal line of code.
 */
Blockly.lua.scrubNakedValue = function(line)
{
	return line + '\n';
};

/**
 * Encode a string as a properly escaped Python string, complete with quotes.
 * @param {string} string Text to encode.
 * @return {string} Python string.
 * @private
 */
Blockly.lua.quote_ = function(string)
{
	// TODO: This is a quick hack.  Replace with goog.string.quote
	string = string.replace('/\\/g', '\\\\')
	         .replace('/\n/g', '\\\n')
	         .replace('/\%/g', '\\%')
	         .replace('/\'/g', '\\\'');
	return '\'' + string + '\'';
};

/**
 * Common tasks for generating Python from blocks.
 * Handles comments for the specified block and any connected value blocks.
 * Calls any statements following this block.
 * @param {!Blockly.Block} block The current block.
 * @param {string} code The Python code created for this block.
 * @return {string} Python code with comments and subsequent blocks added.
 * @private
 */
Blockly.lua.scrub_ = function(block, code)
{
	if(code === null)
	{
		// Block has handled code generation itself.
		return '';
	}
	var commentCode = '';
	// Only collect comments for blocks that aren't inline.
	if(!block.outputConnection || !block.outputConnection.targetConnection)
	{
		// Collect comment for this block.
		var comment = block.getCommentText();
		if(comment)
		{
			commentCode += Blockly.Generator.prefixLines(comment, '-- ') + '\n';
		}
		// Collect comments for all value arguments.
		// Don't collect comments for nested statements.
		for(var x = 0; x < block.inputList.length; x++)
		{
			if(block.inputList[x].type == Blockly.INPUT_VALUE)
			{
				var childBlock = block.inputList[x].targetBlock();
				if(childBlock)
				{
					var comment = Blockly.Generator.allNestedComments(childBlock);
					if(comment)
					{
						commentCode += Blockly.Generator.prefixLines(comment, '-- ');
					}
				}
			}
		}
	}
	var nextBlock = block.nextConnection && block.nextConnection.targetBlock();
	var nextCode = this.blockToCode(nextBlock);
	return commentCode + code + nextCode;
};
