
/**
 * @fileoverview Helper functions for generating Lua for blocks.
 * @author Loïc HAMOT
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to language files.
 */
'use strict';

goog.provide('Blockly.Lua');

goog.require('Blockly.Generator');


Blockly.Lua = new Blockly.Generator('Lua');

/**
 * List of illegal variable names.
 * This is not intended to be a security feature.  Blockly is 100% client-side,
 * so bypassing this list is trivial.  This is intended to prevent users from
 * accidentally clobbering a built-in object or function.
 * @private
 */
Blockly.Lua.addReservedWords(
  // http://docs.python.org/reference/lexical_analysis.html#keywords
  'and,break,do,else,elseif,' +
  'end,false,for,function,if,' +
  'in,local,nil,not,or,' +
  'repeat,return,then,true,until,while');

/**
 * Order of operation ENUMs.
 * http://www.lua.org/manual/5.1/manual.html#2.5.6
 */
Blockly.Lua.ORDER_ATOMIC = 0;         // 0 "" ...
Blockly.Lua.ORDER_MEMBER = 1;         // . []
Blockly.Lua.ORDER_HIGH = 2             
Blockly.Lua.ORDER_FUNCTION_CALL = 3;  // Function calls, tables[]
Blockly.Lua.ORDER_EXPONENTIATION = 4; // ^
Blockly.Lua.ORDER_LOGICAL_NOT = 5;    // not # -
Blockly.Lua.ORDER_MULTIPLICATIVE = 6; // * / %
Blockly.Lua.ORDER_UNARY_SIGN = 7;     // + -
Blockly.Lua.ORDER_ADDITIVE = 8;       // + -
Blockly.Lua.ORDER_CONCATENATION = 9;  // ..
Blockly.Lua.ORDER_RELATIONAL = 10;    // <     >     <=    >=    ~=    ==
Blockly.Lua.ORDER_LOGICAL_AND = 11;   // and
Blockly.Lua.ORDER_LOGICAL_OR = 12;    // or
Blockly.Lua.ORDER_NONE = 99;          // (...)

/**
 * Arbitrary code to inject into locations that risk causing infinite loops.
 * Any instances of '%1' will be replaced by the block ID that failed.
 * E.g. '  checkTimeout(%1)\n'
 * @type ?string
 */
Blockly.Lua.INFINITE_LOOP_TRAP = null;

/**
 * Initialise the database of variable names.
 */
Blockly.Lua.init = function()
{
	// Create a dictionary of definitions to be printed before the code.
	Blockly.Lua.definitions_ = Object.create(null);
    // Create a dictionary mapping desired function names in definitions_
    // to actual function names (to avoid collisions with user functions).
    Blockly.Lua.functionNames_ = Object.create(null);

	if(Blockly.Variables)
    {
		if(!Blockly.Lua.variableDB_)
        {
			Blockly.Lua.variableDB_ =
			  new Blockly.Names(Blockly.Lua.RESERVED_WORDS_);
		} 
        else 
        {
			Blockly.Lua.variableDB_.reset();
		}

		var defvars = [];
		var variables = Blockly.Variables.allVariables();
		for(var x = 0; x < variables.length; x++) 
        {
			defvars[x] = Blockly.Lua.variableDB_.getName(variables[x],
			             Blockly.Variables.NAME_TYPE) + ' = nil';
		}
		Blockly.Lua.definitions_['variables'] = defvars.join('\n');
	}
};

/**
 * Prepend the generated code with the variable definitions.
 * @param {string} code Generated code.
 * @return {string} Completed code.
 */
Blockly.Lua.finish = function(code)
{
	// Convert the definitions dictionary into a list.
	var definitions = [];
	for(var name in Blockly.Lua.definitions_)
    {
		definitions.push(Blockly.Lua.definitions_[name]);
	}
	return definitions.join('\n') + '\n\n' + code;
};

/**
 * Naked values are top-level blocks with outputs that aren't plugged into
 * anything.
 * @param {string} line Line of generated code.
 * @return {string} Legal line of code.
 */
Blockly.Lua.scrubNakedValue = function(line)
{
	return line + '\n';
};

/**
 * Encode a string as a properly escaped Python string, complete with quotes.
 * @param {string} string Text to encode.
 * @return {string} Python string.
 * @private
 */
Blockly.Lua.quote_ = function(string)
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
 * @this {Blockly.CodeGenerator}
 * @private
 */
Blockly.Lua.scrub_ = function (block, code) {
  if (code === null) {
    // Block has handled code generation itself.
    return '';
  }
  var commentCode = '';
  // Only collect comments for blocks that aren't inline.
  if (!block.outputConnection || !block.outputConnection.targetConnection) {
    // Collect comment for this block.
    var comment = block.getCommentText();
    if (comment) {
      commentCode += this.prefixLines(comment, '-- ') + '\n';
    }
    // Collect comments for all value arguments.
    // Don't collect comments for nested statements.
    for (var x = 0; x < block.inputList.length; x++) {
      if (block.inputList[x].type == Blockly.INPUT_VALUE) {
        var childBlock = block.inputList[x].connection.targetBlock();
        if (childBlock) {
          var comment = this.allNestedComments(childBlock);
          if (comment) {
            commentCode += this.prefixLines(comment, '-- ');
          }
        }
      }
    }
  }
  var nextBlock = block.nextConnection && block.nextConnection.targetBlock();
  var nextCode = this.blockToCode(nextBlock);
  return commentCode + code + nextCode;
};

/**
 * Define a function to be included in the generated code.
 * The first time this is called with a given desiredName, the code is
 * saved and an actual name is generated.  Subsequent calls with the
 * same desiredName have no effect but have the same return value.
 *
 * It is up to the caller to make sure the same desiredName is not
 * used for different code values.
 *
 * The code gets output when Blockly.Lua.finish() is called.
 *
 * @param {string} desiredName The desired name of the function (e.g., isPrime).
 * @param {code} A list of Lua statements.
 * @return {string} The actual name of the new function.  This may differ
 *     from desiredName if the former has already been taken by the user.
 * @private
 */
Blockly.Lua.provideFunction_ = function(desiredName, code) {
  if (!Blockly.Lua.definitions_[desiredName]) {
    var functionName = Blockly.Lua.variableDB_.getDistinctName(
        desiredName, Blockly.Generator.NAME_TYPE);
    Blockly.Lua.functionNames_[desiredName] = functionName;
    Blockly.Lua.definitions_[desiredName] = code.join('\n').replace(
        Blockly.Lua.FUNCTION_NAME_PLACEHOLDER_REGEXP_, functionName);
  }
  return Blockly.Lua.functionNames_[desiredName];
};
