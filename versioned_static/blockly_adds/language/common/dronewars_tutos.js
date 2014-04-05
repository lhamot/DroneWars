
function getShipDropDown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_SHIP1, 'Mosquito'],
	   //[Blockly.LANG_DRONEWARS_SHIP2, 'Hornet'],
	   //[Blockly.LANG_DRONEWARS_SHIP3, 'Vulture'],
	   //[Blockly.LANG_DRONEWARS_SHIP4, 'Dragon'],
	   //[Blockly.LANG_DRONEWARS_SHIP5, 'Behemoth'],
	   //[Blockly.LANG_DRONEWARS_SHIP6, 'Azathoth'],
	   [Blockly.LANG_DRONEWARS_SHIP7, 'Queen'],
	   //[Blockly.LANG_DRONEWARS_SHIP8, 'Cargo'],
	   //[Blockly.LANG_DRONEWARS_SHIP9, 'LargeCargo']
    ]);
}

function getCannonDropdown() {
  return new Blockly.FieldDropdown(
    [[Blockly.LANG_DRONEWARS_CANNON1, 'Cannon1'],
     //[Blockly.LANG_DRONEWARS_CANNON2, 'Cannon2'],
     //[Blockly.LANG_DRONEWARS_CANNON3, 'Cannon3'],
     //[Blockly.LANG_DRONEWARS_CANNON4, 'Cannon4'],
     //[Blockly.LANG_DRONEWARS_CANNON5, 'Cannon5'],
     //[Blockly.LANG_DRONEWARS_CANNON6, 'Cannon6']
    ]);
}
