'use strict';

goog.provide('Blockly.Lua');

//******************   Fleet    **********************************************
addProperty('Player', 'id', 'Number', 
  Blockly.LANG_DRONEWARS_PLAYER_ID,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'login', 'String', 
  Blockly.LANG_DRONEWARS_PLAYER_LOGIN,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'mainPlanet', 'Coord', 
  Blockly.LANG_DRONEWARS_PLAYER_MAINPLANET,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'score', 'Number', 
  Blockly.LANG_DRONEWARS_PLAYER_SCORE,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'alliance', 'Alliance', 
  Blockly.LANG_DRONEWARS_PLAYER_ALLIANCE,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'planetCount', 'Number', 
  Blockly.LANG_DRONEWARS_PLAYER_PLANETCOUNT,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);
addProperty('Player', 'fleetCount', 'Number', 
  Blockly.LANG_DRONEWARS_PLAYER_FLEETCOUNT,
  Blockly.LANG_CATEGORY_DRONEWARS_PLAYER);            

//getMaxFleetCount
Blockly.Blocks['dronewars_Player_getMaxFleetCount'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLAYER,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('PLAYER')
            .setCheck('Player')
            .appendField(Blockly.LANG_DRONEWARS_PLAYER_MAXFLEETCOUNT);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_Fleet_getMaxFleetCount'] = function(block)
{
    var player = 
        Blockly.Lua.valueToCode(block, 'PLAYER', Blockly.Lua.ORDER_NONE) || 'nil';
    return [player + ':getMaxFleetCount()', Blockly.Lua.ORDER_HIGH];
};

//getMaxPlanetCount
Blockly.Blocks['dronewars_Player_getMaxPlanetCount'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLAYER,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('PLAYER')
            .setCheck('Player')
            .appendField(Blockly.LANG_DRONEWARS_PLAYER_MAXPLANETCOUNT);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_Player_getMaxPlanetCount'] = function(block)
{
    var player = 
        Blockly.Lua.valueToCode(block, 'PLAYER', Blockly.Lua.ORDER_NONE) || 'nil';
    return [player + ':getMaxPlanetCount()', Blockly.Lua.ORDER_HIGH];
};

//getMaxFleetSize
Blockly.Blocks['dronewars_Player_getMaxFleetSize'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLAYER,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('PLAYER')
            .setCheck('Player')
            .appendField(Blockly.LANG_DRONEWARS_PLAYER_MAXFLEETSIZE);
        this.setOutput(true, 'Number');
    }
};

Blockly.Lua['dronewars_Player_getMaxFleetSize'] = function(block)
{
    var player = 
        Blockly.Lua.valueToCode(block, 'PLAYER', Blockly.Lua.ORDER_NONE) || 'nil';
    return [player + ':getMaxFleetSize()', Blockly.Lua.ORDER_HIGH];
};

//log
Blockly.Blocks['dronewars_log'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLAYER,
init:
    function()
    {
        this.setColour(230);
        this.appendValueInput('MESSAGE')
            .appendField(Blockly.LANG_DRONEWARS_LOG);
        //this.interpolateMsg(Blockly.LANG_DRONEWARS_LOG,
        //                    ['MESSAGE', null, Blockly.ALIGN_RIGHT],
        //                    Blockly.ALIGN_RIGHT);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
    }
};

Blockly.Lua['dronewars_log'] = function(block)
{
    var message = Blockly.Lua.valueToCode(block, 'MESSAGE', Blockly.Lua.ORDER_NONE) || 'nil';
    return 'log(' + message + ')\n';
};

//currentPlayer
Blockly.Blocks['dronewars_currentPlayer'] =
{
category:
    Blockly.LANG_CATEGORY_DRONEWARS_PLAYER,
init:
    function()
    {
        this.setColour(230);
        this.appendDummyInput()
            .appendField(Blockly.LANG_DRONEWARS_CURRENTPLAYER)
        this.setOutput(true, 'Player');
    }
};

Blockly.Lua['dronewars_currentPlayer'] = function(block)
{
    var message = Blockly.Lua.valueToCode(block, 'MESSAGE', Blockly.Lua.ORDER_NONE) || 'nil';
    return ['currentPlayer', Blockly.Lua.ORDER_HIGH];
};

