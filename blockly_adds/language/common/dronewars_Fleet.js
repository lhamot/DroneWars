
if(!Blockly.Language) Blockly.Language = {};


//******************   Fleet    **********************************************
addProperty('Fleet', 'id', Number, Blockly.LANG_DRONEWARS_FLEET_ID,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'playerId', Number, Blockly.LANG_DRONEWARS_FLEET_PLAYER_ID,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'coord', 'Coord', Blockly.LANG_DRONEWARS_FLEET_COORD,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'origin', 'Coord', Blockly.LANG_DRONEWARS_FLEET_ORIGIN,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'name', String, Blockly.LANG_DRONEWARS_FLEET_NAME,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'shipList', Array, Blockly.LANG_DRONEWARS_FLEET_SHIPLIST,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);
addProperty('Fleet', 'ressourceSet', 'RessourceSet', Blockly.LANG_DRONEWARS_FLEET_RESSOURCESET,
  Blockly.LANG_CATEGORY_DRONEWARS_FLEET);

//ShipList
Blockly.Language.dronewars_ship_in_fleet =
{
category:
	Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
init:
	function()
	{
		this.setColour(230);
		this.setInputsInline(true);
		this.appendDummyInput()
      .appendTitle(getShipDropDown(), 'SHIP');
	  this.appendValueInput('FLEET')
      .setCheck('Fleet')
      .appendTitle(Blockly.LANG_DRONEWARS_SHIP_IN_FLEET);
		this.setOutput(true, Number);
	}
};

Blockly.lua.dronewars_ship_in_fleet = function()
{
	var object = Blockly.lua.valueToCode(
	               this, 'FLEET', Blockly.lua.ORDER_NONE) || 'nil';
	return [object + '.shipList[Ship.' + this.getTitleValue('SHIP') + ']',
	        Blockly.lua.ORDER_FUNCTION_CALL
	       ];
};

//ShipList
Blockly.Language.dronewars_ressource_in_fleet =
{
  category:
    Blockly.LANG_CATEGORY_DRONEWARS_FLEET,
  init:
    function () {
      this.setColour(230);
      this.setInputsInline(true);
      this.appendDummyInput()
        .appendTitle(getRessourceDropDown(), 'RESSOURCE');
      this.appendValueInput('FLEET')
        .setCheck('Fleet')
        .appendTitle(Blockly.LANG_DRONEWARS_RESSOURCE_IN_FLEET);
      this.setOutput(true, Number);
    }
};

Blockly.lua.dronewars_ressource_in_fleet = function () {
  var object = Blockly.lua.valueToCode(
	               this, 'FLEET', Blockly.lua.ORDER_NONE) || 'nil';
  return [object + '.ressourceSet:at(Ressource.' + this.getTitleValue('RESSOURCE') + ')',
	        Blockly.lua.ORDER_FUNCTION_CALL
  ];
};

