
if(!Blockly.Language) Blockly.Language = {};

function checkNumber(text)
{
	var n = window.parseFloat(text || 0);
	return window.isNaN(n) ? null : String(n);
}

function getShipDropDown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_SHIP1, 'Mosquito'],
	   [Blockly.LANG_DRONEWARS_SHIP2, 'Hornet'],
	   [Blockly.LANG_DRONEWARS_SHIP3, 'Vulture'],
	   [Blockly.LANG_DRONEWARS_SHIP4, 'Dragon'],
	   [Blockly.LANG_DRONEWARS_SHIP5, 'Behemoth'],
	   [Blockly.LANG_DRONEWARS_SHIP6, 'Azathoth'],
	   [Blockly.LANG_DRONEWARS_SHIP7, 'Queen'],
	   [Blockly.LANG_DRONEWARS_SHIP8, 'Cargo'],
	   [Blockly.LANG_DRONEWARS_SHIP9, 'LargeCargo']
    ]);
}

function getCannonDropdown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_CANNON1, 'Cannon1'],
     [Blockly.LANG_DRONEWARS_CANNON2, 'Cannon2'],
     [Blockly.LANG_DRONEWARS_CANNON3, 'Cannon3'],
	   [Blockly.LANG_DRONEWARS_CANNON4, 'Cannon4'],
	   [Blockly.LANG_DRONEWARS_CANNON5, 'Cannon5'],
	   [Blockly.LANG_DRONEWARS_CANNON6, 'Cannon6']
    ]);
}

function getFleetActionDropdown()
{
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_FLEETACTION1, 'Nothing'],
	   [Blockly.LANG_DRONEWARS_FLEETACTION2, 'Move'],
	   [Blockly.LANG_DRONEWARS_FLEETACTION3, 'Harvest'],
	   [Blockly.LANG_DRONEWARS_FLEETACTION4, 'Colonize'],
	   [Blockly.LANG_DRONEWARS_FLEETACTION5, 'Drop']
    ]);
}

function addProperty(className, propName, returnType, label, categ) {
  jsFuncName = 'dronewars_' + className + '_' + propName;
  upperClassName = className.toUpperCase();
  Blockly.Language[jsFuncName] =
	{
	  category:
        categ,//Blockly.LANG_CATEGORY_DRONEWARS,
	  init:
        function () {
          this.setColour(230);
          //this.setInputsInline(true);
          this.appendInput(label,
          Blockly.INPUT_VALUE, upperClassName, className);
          this.setOutput(true, returnType);
        }
	};

  Blockly.lua[jsFuncName] = function () {
    var object = Blockly.lua.valueToCode(
		               this, upperClassName, Blockly.JavaScript.ORDER_NONE) || 'nil';
    return [object + '.' + propName, Blockly.lua.ORDER_FUNCTION_CALL];
  };
}