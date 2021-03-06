

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
	 //[Blockly.LANG_DRONEWARS_SHIP8, 'Cargo'],
	 //[Blockly.LANG_DRONEWARS_SHIP9, 'LargeCargo']
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

function getBuildingDropdown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_BUILDING1, 'CommandCenter'],
     [Blockly.LANG_DRONEWARS_BUILDING2, 'MetalMine'],
     //[Blockly.LANG_DRONEWARS_BUILDING3, 'CarbonMine'],
     //[Blockly.LANG_DRONEWARS_BUILDING4, 'LoiciumFilter'],
     [Blockly.LANG_DRONEWARS_BUILDING5, 'Factory'],
     //[Blockly.LANG_DRONEWARS_BUILDING6, 'Laboratory'],
     //[Blockly.LANG_DRONEWARS_BUILDING7, 'CarbonicCentral'],
     //[Blockly.LANG_DRONEWARS_BUILDING8, 'SolarCentral'],
     //[Blockly.LANG_DRONEWARS_BUILDING9, 'GeothermicCentral']
    ]);
}


function getRessourceDropDown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_RESSOURCE1, 'Metal'],
	   [Blockly.LANG_DRONEWARS_RESSOURCE2, 'Carbon'],
	   [Blockly.LANG_DRONEWARS_RESSOURCE3, 'Loicium'],
    ]);
}


function addProperty(className, propName, returnType, label, categ) {
  jsFuncName = 'dronewars_' + className + '_' + propName;
  upperClassName = className.toUpperCase();
  Blockly.Blocks[jsFuncName] =
	{
	  category:
        categ,//Blockly.LANG_CATEGORY_DRONEWARS,
	  init:
        function () {
          this.setColour(230);
          //this.setInputsInline(true);
          this.appendValueInput(upperClassName)
            .setCheck(className)
            .appendField(label);
          this.setOutput(true, returnType);
        }
	};

  Blockly.Lua[jsFuncName] = function () {
    var object = Blockly.Lua.valueToCode(
		               this, upperClassName, Blockly.Lua.ORDER_NONE) || 'nil';
    return [object + '.' + propName, Blockly.Lua.ORDER_HIGH];
  };
}



Blockly.Blocks['procedures_return'] = {
  init: function() {
    this.setColour(290);
    this.appendValueInput('VALUE')
        .appendField(Blockly.LANG_PROCEDURES_DEFRETURN_RETURN);
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    //this.setTooltip(Blockly.LANG_PROCEDURES_RETURN_TOOLTIP);
    this.hasReturnValue_ = true;
  },
  mutationToDom: function() {
    // Save whether this block has a return value.
    var container = document.createElement('mutation');
    container.setAttribute('value', Number(this.hasReturnValue_));
    return container;
  },
  domToMutation: function(xmlElement) {
    // Restore whether this block has a return value.
    var value = xmlElement.getAttribute('value');
    this.hasReturnValue_ = (value == 1);
    if (!this.hasReturnValue_) {
      this.removeInput('VALUE');
      this.appendDummyInput('VALUE')
        .appendField(Blockly.LANG_PROCEDURES_DEFRETURN_RETURN);
    }
  },
  onchange: function() {
    if (!this.workspace) {
      // Block has been deleted.
      return;
    }
    var legal = false;
    // Is the block nested in a procedure?
    var block = this;
    do {
      if (block.type == 'procedures_defnoreturn' ||
          block.type == 'procedures_defreturn') {
        legal = true;
        break;
      }
      block = block.getSurroundParent();
    } while (block);
    if (legal) {
      // If needed, toggle whether this block has a return value.
      if (block.type == 'procedures_defnoreturn' && this.hasReturnValue_) {
        this.removeInput('VALUE');
        this.appendDummyInput('VALUE')
          .appendField(Blockly.LANG_PROCEDURES_DEFRETURN_RETURN);
        this.hasReturnValue_ = false;
      } else if (block.type == 'procedures_defreturn' &&
                 !this.hasReturnValue_) {
        this.removeInput('VALUE');
        this.appendValueInput('VALUE')
          .appendField(Blockly.LANG_PROCEDURES_DEFRETURN_RETURN);
        this.hasReturnValue_ = true;
      }
      this.setWarningText(null);
    } else {
      this.setWarningText(Blockly.LANG_PROCEDURES_IFRETURN_WARNING);
    }
  }
};

