// Text strings (factored out to make multi-language easier).

/**
 * Due to the frequency of long strings, the 80-column wrap rule need not apply
 * to message files.
 */

// Context menus.
Blockly.MSG_DUPLICATE_BLOCK = 'Dupliquer';
Blockly.MSG_REMOVE_COMMENT = 'Retirer commentaire';
Blockly.MSG_ADD_COMMENT = 'Ajouter commentaire';
Blockly.MSG_EXTERNAL_INPUTS = 'Entrées a droite';
Blockly.MSG_INLINE_INPUTS = 'Entrées en ligne';
Blockly.MSG_DELETE_BLOCK = 'Supprime bloque';
Blockly.MSG_DELETE_X_BLOCKS = 'Supprime le bloque "%1"';
Blockly.MSG_COLLAPSE_BLOCK = 'Réduit le bloque';
Blockly.MSG_EXPAND_BLOCK = 'Etant de bloque';
Blockly.MSG_DISABLE_BLOCK = 'Désactive le bloque';
Blockly.MSG_ENABLE_BLOCK = 'Active le bloque';
Blockly.MSG_HELP = 'Aide';

// Variable renaming.
Blockly.MSG_CHANGE_VALUE_TITLE = 'Modifier valeur :';
Blockly.MSG_NEW_VARIABLE = 'Nouvelle variable...';
Blockly.MSG_NEW_VARIABLE_TITLE = 'Nom de la nouvelle variable :';
Blockly.MSG_RENAME_VARIABLE = 'Renomer variable...';
Blockly.MSG_RENAME_VARIABLE_TITLE = 'Renomer toute les "%1" variables par :';

// Toolbox.
Blockly.MSG_VARIABLE_CATEGORY = 'Variables';
Blockly.MSG_PROCEDURE_CATEGORY = 'Procedures';

// Control Blocks
Blockly.LANG_CATEGORY_CONTROLS = 'Controle';
Blockly.LANG_COLOUR_PICKER_HELPURL = 'http://en.wikipedia.org/wiki/Color';
Blockly.LANG_COLOUR_PICKER_TOOLTIP = 'Choisir une couleur dans la palète.';

Blockly.LANG_COLOUR_RGB_HELPURL = 'http://en.wikipedia.org/wiki/RGB_color_model';
Blockly.LANG_COLOUR_RGB_TITLE = 'couleur avec';
Blockly.LANG_COLOUR_RGB_RED = 'rouge';
Blockly.LANG_COLOUR_RGB_GREEN = 'vert';
Blockly.LANG_COLOUR_RGB_BLUE = 'bleu';
Blockly.LANG_COLOUR_RGB_TOOLTIP =  'Creer une couleur avec une quantité ' +
               'spécifiée\nde rouge, vert et bleu. Toutes les valeurs doivent ètre entre 0.0 et 1.0';

Blockly.LANG_COLOUR_BLEND_HELPURL = 'http://meyerweb.com/eric/tools/color-blend/';
Blockly.LANG_COLOUR_BLEND_TITLE = 'blend';
Blockly.LANG_COLOUR_BLEND_COLOUR1 = 'colour 1';
Blockly.LANG_COLOUR_BLEND_COLOUR2 = 'colour 2';
Blockly.LANG_COLOUR_BLEND_RATIO = 'ratio';
Blockly.LANG_COLOUR_BLEND_TOOLTIP = 'Blends two colours together with a given ratio (0.0 - 1.0).';

// Control Blocks.
Blockly.LANG_CATEGORY_CONTROLS = 'Control';
Blockly.LANG_CONTROLS_IF_HELPURL = 'http://code.google.com/p/blockly/wiki/If_Then';
Blockly.LANG_CONTROLS_IF_TOOLTIP_1 = "Si c'est vraie, alors excecute certaines instructions";//'If a value is true, then do some statements.';
Blockly.LANG_CONTROLS_IF_TOOLTIP_2 = "Si c'est vraie, alors excecute le premier bloque d'instruction.\n" +//'If a value is true, then do the first block of statements.\n' +
               "Sinon, excecute le second bloque d'instructions.";//'Otherwise, do the second block of statements.';
Blockly.LANG_CONTROLS_IF_TOOLTIP_3 = "Si la première valeur est vraie, alors excecute le premiuer bloque d'instructions.\n" +//'If the first value is true, then do the first block of statements.\n' +
               "Sinon, si le second est vraie, excecute le second bloque d'instructions.";//'Otherwise, if the second value is true, do the second block of statements.';
Blockly.LANG_CONTROLS_IF_TOOLTIP_4 = "Si la premère valeur est vraie, alors excecute le premier bloque d'instructions.\n" +//'If the first value is true, then do the first block of statements.\n' +
               "Sinon, si la seconds valeur est vraie, excecute le second bloque d'instructions.\n" + //'Otherwise, if the second value is true, do the second block of statements.\n' +
               "Si aucunes valeurs n'est vraie, excecute le dernier bloque d'instructions.";//'If none of the values are true, do the last block of statements.';
Blockly.LANG_CONTROLS_IF_MSG_IF = 'si';
Blockly.LANG_CONTROLS_IF_MSG_ELSEIF = 'sinon si';
Blockly.LANG_CONTROLS_IF_MSG_ELSE = 'sinon';
Blockly.LANG_CONTROLS_IF_MSG_THEN = 'alors';

Blockly.LANG_CONTROLS_IF_IF_TITLE_IF = 'si';
Blockly.LANG_CONTROLS_IF_IF_TOOLTIP = 'Ajoute, retire, ou réordonne les sections\n' + //'Add, remove, or reorder sections\n' +
                    "pour reconfigurer ce bloque 'si'";//'to reconfigure this if block.';

Blockly.LANG_CONTROLS_IF_ELSEIF_TITLE_ELSEIF = 'sinon si';
Blockly.LANG_CONTROLS_IF_ELSEIF_TOOLTIP = "Ajoute une condition dans le bloque 'si'.";//'Add a condition to the if block.';

Blockly.LANG_CONTROLS_IF_ELSE_TITLE_ELSE = 'sinon';
Blockly.LANG_CONTROLS_IF_ELSE_TOOLTIP = "Ajoute une condition final, par defaut, au bloque 'si'";//'Add a final, catch-all condition to the if block.';

Blockly.LANG_CONTROLS_REPEAT_HELPURL = 'http://en.wikipedia.org/wiki/For_loop';
Blockly.LANG_CONTROLS_REPEAT_TITLE_REPEAT = 'répete';
Blockly.LANG_CONTROLS_REPEAT_TITLE_TIMES = 'fois';
Blockly.LANG_CONTROLS_REPEAT_INPUT_DO = 'fait';
Blockly.LANG_CONTROLS_REPEAT_TOOLTIP = 'Do some statements several times.';

Blockly.LANG_CONTROLS_WHILEUNTIL_HELPURL = 'http://code.google.com/p/blockly/wiki/Repeat';
Blockly.LANG_CONTROLS_WHILEUNTIL_TITLE_REPEAT = 'répète';
Blockly.LANG_CONTROLS_WHILEUNTIL_INPUT_DO = 'fait';
Blockly.LANG_CONTROLS_WHILEUNTIL_OPERATOR_WHILE = 'tant que';
Blockly.LANG_CONTROLS_WHILEUNTIL_OPERATOR_UNTIL = "jusqu'à ce que";
Blockly.LANG_CONTROLS_WHILEUNTIL_TOOLTIP_WHILE = "Tant qu'une valeur est vraie, alors excecute certaines instructions.";//'While a value is true, then do some statements.';
Blockly.LANG_CONTROLS_WHILEUNTIL_TOOLTIP_UNTIL = "Tant qu'une valeur est fausse, alors excecute certaines instructions.";//'While a value is false, then do some statements.';

Blockly.LANG_CONTROLS_FOR_HELPURL = 'http://en.wikipedia.org/wiki/For_loop';
Blockly.LANG_CONTROLS_FOR_INPUT_WITH = 'pour';
Blockly.LANG_CONTROLS_FOR_INPUT_VAR = 'x';
Blockly.LANG_CONTROLS_FOR_INPUT_FROM = 'compris entre';
Blockly.LANG_CONTROLS_FOR_INPUT_TO = 'et';
Blockly.LANG_CONTROLS_FOR_INPUT_DO = 'fait';
Blockly.LANG_CONTROLS_FOR_TOOLTIP =
    "Enumére les nombres d'un debut donnée a une fin donnée.\n" +//'Count from a start number to an end number.\n' +
    "Pour chaque nombre, change la variable '%1' à ce nombre,\n" +//'For each count, set the current count number to\n' +
    "et excecute certaines instructions.";//'variable "%1", and then do some statements.';

Blockly.LANG_CONTROLS_FOREACH_HELPURL = 'http://en.wikipedia.org/wiki/For_loop';
Blockly.LANG_CONTROLS_FOREACH_INPUT_ITEM = 'pour chaque élément';
Blockly.LANG_CONTROLS_FOREACH_INPUT_VAR = 'x';
Blockly.LANG_CONTROLS_FOREACH_INPUT_INLIST = 'dans la liste';
Blockly.LANG_CONTROLS_FOREACH_INPUT_DO = 'fait';
Blockly.LANG_CONTROLS_FOREACH_TOOLTIP =
  "Pour chaque élément dans la liste, positionne la variable '%1' sur l'élément,\n" +//'For each item in a list, set the item to\n' +
  "et exécute certaines instructions.";//'variable "%1", and then do some statements.';

Blockly.LANG_CONTROLS_FLOW_STATEMENTS_HELPURL = 'http://en.wikipedia.org/wiki/Control_flow';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_INPUT_OFLOOP = 'la boucle';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_OPERATOR_BREAK = 'sort de';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_OPERATOR_CONTINUE = "continue";//'continue with next iteration';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_TOOLTIP_BREAK = 'Sort de la boucle actuelle.';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_TOOLTIP_CONTINUE =
    "Passe le reste de la boucle, \n" +//'Skip the rest of this loop, and\n' +
    "et continue a la prochaine iteration.";//'continue with the next iteration.';
Blockly.LANG_CONTROLS_FLOW_STATEMENTS_WARNING = "Attention:\n" +//'Warning:\n' +
    "Ce bloque peu seulement\n" + //'This block may only\n' +
    "ètre utilisé dans une boucle.";//'be used within a loop.';

// Logic Blocks.
Blockly.LANG_CATEGORY_LOGIC = 'Logic';
Blockly.LANG_LOGIC_COMPARE_HELPURL = 'http://en.wikipedia.org/wiki/Inequality_(mathematics)';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_EQ = "Retourne vrai if les deux arguments sont égaux.";//'Return true if both inputs equal each other.';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_NEQ = "Retourne vrai si les deux arguments sont differents.";//'Return true if both inputs are not equal to each other.';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_LT = "Retourne vrai si le premier argument est\n" + //'Return true if the first input is smaller\n' +
      "plus petit que le second";//'than the second input.';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_LTE =
       "Retourne vrai si le premier argument est\n" + //'Return true if the first input is smaller\n' +
       "plus petit ou égal au second.";//'than or equal to the second input.';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_GT =
      "Retourne vrai si le premier agument est\n" +//'Return true if the first input is greater\n' +
      "plus grand que le second.";//'than the second input.';
Blockly.LANG_LOGIC_COMPARE_TOOLTIP_GTE =
       "Retourne vrai si le premier argument est\n" +//'Return true if the first input is greater\n' +
       "plus grand ou egale au second.";//'than or equal to the second input.';

Blockly.LANG_LOGIC_OPERATION_HELPURL = 'http://code.google.com/p/blockly/wiki/And_Or';
Blockly.LANG_LOGIC_OPERATION_AND = 'et';
Blockly.LANG_LOGIC_OPERATION_OR = 'ou';
Blockly.LANG_LOGIC_OPERATION_TOOLTIP_AND = "Retourne vrai si les deux arguments sont vrai.";//'Return true if both inputs are true.';
Blockly.LANG_LOGIC_OPERATION_TOOLTIP_OR = "Retourne vrai si au moins un argument est vrai.";//'Return true if either inputs are true.';

Blockly.LANG_LOGIC_NEGATE_HELPURL = 'http://code.google.com/p/blockly/wiki/Not';
Blockly.LANG_LOGIC_NEGATE_INPUT_NOT = 'pas';
Blockly.LANG_LOGIC_NEGATE_TOOLTIP = "Retourne vrai si l'argument est faux.\n" + //'Returns true if the input is false.\n' +
                    "Retourne faux si l\'argument est vrai.";

Blockly.LANG_LOGIC_BOOLEAN_HELPURL = 'http://code.google.com/p/blockly/wiki/True_False';
Blockly.LANG_LOGIC_BOOLEAN_TRUE = 'vrai';
Blockly.LANG_LOGIC_BOOLEAN_FALSE = 'faux';
Blockly.LANG_LOGIC_BOOLEAN_TOOLTIP = "Retourne vrai ou faux";//'Returns either true or false.';

Blockly.LANG_LOGIC_NULL_HELPURL = 'http://en.wikipedia.org/wiki/Nullable_type';
Blockly.LANG_LOGIC_NULL = 'null';
Blockly.LANG_LOGIC_NULL_TOOLTIP = 'Retourne null.';

Blockly.LANG_LOGIC_TERNARY_HELPURL = 'http://en.wikipedia.org/wiki/%3F:';
Blockly.LANG_LOGIC_TERNARY_CONDITION = 'teste';
Blockly.LANG_LOGIC_TERNARY_IF_TRUE = 'si vrai';
Blockly.LANG_LOGIC_TERNARY_IF_FALSE = 'si faux';
Blockly.LANG_LOGIC_TERNARY_TOOLTIP = 'Check the condition in "test". If the condition is true\n' +
    'returns the "if true" value, otherwise returns the "if false" value.';

// Math Blocks.
Blockly.LANG_CATEGORY_MATH = 'Math';
Blockly.LANG_MATH_NUMBER_HELPURL = 'http://en.wikipedia.org/wiki/Number';
Blockly.LANG_MATH_NUMBER_TOOLTIP = "Un nombre.";//'A number.';

Blockly.LANG_MATH_ARITHMETIC_HELPURL = 'http://en.wikipedia.org/wiki/Arithmetic';
Blockly.LANG_MATH_ARITHMETIC_TOOLTIP_ADD = "Retourne la somme des deux nombres.";//'Return the sum of the two numbers.';
Blockly.LANG_MATH_ARITHMETIC_TOOLTIP_MINUS = "Retourne la difference des deux nombres.";//'Return the difference of the two numbers.';
Blockly.LANG_MATH_ARITHMETIC_TOOLTIP_MULTIPLY = "Retourne le produit des deux nombres.";//'Return the product of the two numbers.';
Blockly.LANG_MATH_ARITHMETIC_TOOLTIP_DIVIDE = "Retourne le quotient des deux nombres.";//'Return the quotient of the two numbers.';
Blockly.LANG_MATH_ARITHMETIC_TOOLTIP_POWER = "Retourne le premier nombre\n" +//'Return the first number raised to\n' +
  "élevé à la puissance du second nombre.";//'the power of the second number.';

Blockly.LANG_MATH_SINGLE_HELPURL = 'http://en.wikipedia.org/wiki/Square_root';
Blockly.LANG_MATH_SINGLE_OP_ROOT = 'racine carrée';
Blockly.LANG_MATH_SINGLE_OP_ABSOLUTE = 'absolue';
Blockly.LANG_MATH_SINGLE_TOOLTIP_ROOT = "Retourne la racine carré d'un nombre.";
Blockly.LANG_MATH_SINGLE_TOOLTIP_ABS = "Retourne la valeur absolue d'un nombre.";
Blockly.LANG_MATH_SINGLE_TOOLTIP_NEG = "Retourne l'opposé d'un nombre.";
Blockly.LANG_MATH_SINGLE_TOOLTIP_LN = "Retourne le logarithme naturel d'un nombre.";
Blockly.LANG_MATH_SINGLE_TOOLTIP_LOG10 = "Retourne le logarithme de base 10 d'un nombre.";
Blockly.LANG_MATH_SINGLE_TOOLTIP_EXP = 'Retourne e élevé a la puissance du nombre.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_POW10 = 'Retourne 10 élevé a la puissance du nombre.';

Blockly.LANG_MATH_TRIG_HELPURL = 'http://en.wikipedia.org/wiki/Trigonometric_functions';
Blockly.LANG_MATH_TRIG_TOOLTIP_SIN = 'Return the sine of a degree (not radian).';
Blockly.LANG_MATH_TRIG_TOOLTIP_COS = 'Return the cosine of a degree (not radian).';
Blockly.LANG_MATH_TRIG_TOOLTIP_TAN = 'Return the tangent of a degree (not radian).';
Blockly.LANG_MATH_TRIG_TOOLTIP_ASIN = 'Return the arcsine of a number.';
Blockly.LANG_MATH_TRIG_TOOLTIP_ACOS = 'Return the arccosine of a number.';
Blockly.LANG_MATH_TRIG_TOOLTIP_ATAN = 'Return the arctangent of a number.';

Blockly.LANG_MATH_CONSTANT_HELPURL = 'http://en.wikipedia.org/wiki/Mathematical_constant';
Blockly.LANG_MATH_CONSTANT_TOOLTIP = 'Return one of the common constants: \u03c0 (3.141\u2026), e (2.718\u2026), \u03c6 (1.618\u2026),\n' +
    'sqrt(2) (1.414\u2026), sqrt(\u00bd) (0.707\u2026), or \u221e (infinity).';

Blockly.LANG_MATH_IS_EVEN = 'is even';
Blockly.LANG_MATH_IS_ODD = 'is odd';
Blockly.LANG_MATH_IS_PRIME = 'is prime';
Blockly.LANG_MATH_IS_WHOLE = 'is whole';
Blockly.LANG_MATH_IS_POSITIVE = 'is positive';
Blockly.LANG_MATH_IS_NEGATIVE = 'is negative';
Blockly.LANG_MATH_IS_DIVISIBLE_BY = 'is divisible by';
Blockly.LANG_MATH_IS_TOOLTIP = 'Check if a number is an even, odd, prime, whole, positive, negative,\n' +
    'or if it is divisible by certain number.  Returns true or false.';

Blockly.LANG_MATH_CHANGE_HELPURL = 'http://en.wikipedia.org/wiki/Negation';
Blockly.LANG_MATH_CHANGE_TITLE_CHANGE = 'dans';
Blockly.LANG_MATH_CHANGE_TITLE_ITEM = 'élément';
Blockly.LANG_MATH_CHANGE_INPUT_BY = 'place';
Blockly.LANG_MATH_CHANGE_TOOLTIP = 'Met un nombre dans la variable "%1".';//'Add a number to variable "%1".';

Blockly.LANG_MATH_SINGLE_HELPURL = 'http://en.wikipedia.org/wiki/Square_root';
Blockly.LANG_MATH_SINGLE_OP_ROOT = 'racine carrée';
Blockly.LANG_MATH_SINGLE_OP_ABSOLUTE = 'valeur absolue';
Blockly.LANG_MATH_SINGLE_TOOLTIP_ROOT = "Retourne la racine carrée d'un nombre.";//'Return the square root of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_ABS = "Retourne la valeur absolue d'un nombre.";//'Return the absolute value of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_NEG = "Retourne l'opposé d'un nombre.";//'Return the negation of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_LN = "Retourne le logarithme naturel d'un nombre."//'Return the natural logarithm of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_LOG10 = "Retourne le logarithme de base 10 d'un nombre.";//'Return the base 10 logarithm of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_EXP = "Retourne e élevé à une puissance donnée.";//'Return e to the power of a number.';
Blockly.LANG_MATH_SINGLE_TOOLTIP_POW10 = "Retourne 10 élevé à une puissance donnée.";//'Return 10 to the power of a number.';

Blockly.LANG_MATH_ROUND_HELPURL = 'http://en.wikipedia.org/wiki/Rounding';
Blockly.LANG_MATH_ROUND_TOOLTIP = "Arrondie un nombre au dessu ou en dessou.";//'Round a number up or down.';
Blockly.LANG_MATH_ROUND_OPERATOR_ROUND = 'arrondi';
Blockly.LANG_MATH_ROUND_OPERATOR_ROUNDUP = 'arrondi au dessu';
Blockly.LANG_MATH_ROUND_OPERATOR_ROUNDDOWN = 'arrondi en dessou';

Blockly.LANG_MATH_TRIG_HELPURL = 'http://en.wikipedia.org/wiki/Trigonometric_functions';
Blockly.LANG_MATH_TRIG_TOOLTIP_SIN = "Retourne le sinus d'un degré.";//'Return the sine of a degree.';
Blockly.LANG_MATH_TRIG_TOOLTIP_COS = "Retourne le cosinus d'un degré.";//'Return the cosine of a degree.';
Blockly.LANG_MATH_TRIG_TOOLTIP_TAN = "Retourne la tangente d'un degré.";//'Return the tangent of a degree.';
Blockly.LANG_MATH_TRIG_TOOLTIP_ASIN = "Retourne l'arcsinus d'un nombre.";//'Return the arcsine of a number.';
Blockly.LANG_MATH_TRIG_TOOLTIP_ACOS = "Retourne l'arccosinus d'un nombre.";//'Return the arccosine of a number.';
Blockly.LANG_MATH_TRIG_TOOLTIP_ATAN = "Retourne l'arctangente d'un nombre.";//'Return the arctangent of a number.';

Blockly.LANG_MATH_ONLIST_HELPURL = '';
Blockly.LANG_MATH_ONLIST_INPUT_OFLIST = "de la liste";
Blockly.LANG_MATH_ONLIST_OPERATOR_SUM = 'somme'; 
Blockly.LANG_MATH_ONLIST_OPERATOR_MIN = 'min';
Blockly.LANG_MATH_ONLIST_OPERATOR_MAX = 'max';
Blockly.LANG_MATH_ONLIST_OPERATOR_AVERAGE = 'moyenne';
Blockly.LANG_MATH_ONLIST_OPERATOR_MEDIAN = 'mediane';
Blockly.LANG_MATH_ONLIST_OPERATOR_MODE = 'modes';
Blockly.LANG_MATH_ONLIST_OPERATOR_STD_DEV = 'écart-type';
Blockly.LANG_MATH_ONLIST_OPERATOR_RANDOM = 'élément au hazard';
Blockly.LANG_MATH_ONLIST_TOOLTIP_SUM = "Retourne la somme de tout les nombre dans la liste.";//'Return the sum of all the numbers in the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_MIN = "Retourne le plus petit nombre de la liste.";//'Return the smallest number in the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_MAX = "Retourne le plus grand nombre de la liste.";//'Return the largest number in the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_AVERAGE = "Retourne la moyenne arithmétique de la liste.";//'Return the arithmetic mean of the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_MEDIAN = "Retourne la valeur médiane de la liste.";//'Return the median number in the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_MODE = "Retourne une liste des valeurs les plus représentées dans la liste.";//'Return a list of the most common item(s) in the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_STD_DEV = "Retourne l'écart-type de la liste.";//'Return the standard deviation of the list.';
Blockly.LANG_MATH_ONLIST_TOOLTIP_RANDOM = "Retourne un élément tiré au hazard dans la liste.";//'Return a random element from the list.';

Blockly.LANG_MATH_MODULO_HELPURL = 'http://en.wikipedia.org/wiki/Modulo_operation';
Blockly.LANG_MATH_MODULO_INPUT_DIVIDEND = 'modulo de';
Blockly.LANG_MATH_MODULO_TOOLTIP = 'Retourne le modulo de la division de deux nombres.';

Blockly.LANG_MATH_CONSTRAIN_HELPURL = 'http://en.wikipedia.org/wiki/Clamping_%28graphics%29';
Blockly.LANG_MATH_CONSTRAIN_INPUT_CONSTRAIN = 'limite';
Blockly.LANG_MATH_CONSTRAIN_INPUT_LOW = 'entre (min)';
Blockly.LANG_MATH_CONSTRAIN_INPUT_HIGH = 'et (max)';
Blockly.LANG_MATH_CONSTRAIN_TOOLTIP = "Limite un nombre entre des bornes données(inclusive).";//'Constrain a number to be between the specified limits (inclusive).';

Blockly.LANG_MATH_MODULO_HELPURL = 'http://en.wikipedia.org/wiki/Modulo_operation';
Blockly.LANG_MATH_MODULO_INPUT_DIVIDEND = 'reste de';
Blockly.LANG_MATH_MODULO_TOOLTIP = "Retourne le reste de la division des deux valeurs.";//'Return the remainder of dividing both numbers.';

Blockly.LANG_MATH_RANDOM_INT_HELPURL = 'http://en.wikipedia.org/wiki/Random_number_generation';
Blockly.LANG_MATH_RANDOM_INT_INPUT_FROM = 'entier au hazard entre';
Blockly.LANG_MATH_RANDOM_INT_INPUT_TO = 'et';
Blockly.LANG_MATH_RANDOM_INT_TOOLTIP = "Retourne une valeur entiere au hazard\n" +
    "entre les bornes données(inclusive)";

Blockly.LANG_MATH_RANDOM_FLOAT_HELPURL = 'http://en.wikipedia.org/wiki/Random_number_generation';
Blockly.LANG_MATH_RANDOM_FLOAT_TITLE_RANDOM = 'fraction au hazard';
Blockly.LANG_MATH_RANDOM_FLOAT_TOOLTIP = "Retourne un nombre decimale aléatoire\n" +
    "entre 0.0 (inclusif) et 1.0 (exclusif).";

// Text Blocks.
Blockly.LANG_CATEGORY_TEXT = 'Texte';
Blockly.LANG_TEXT_TEXT_HELPURL = 'http://en.wikipedia.org/wiki/String_(computer_science)';
Blockly.LANG_TEXT_TEXT_TOOLTIP = "Une lettre, un mot ou ligne de texte.";//'A letter, word, or line of text.';

Blockly.LANG_TEXT_JOIN_HELPURL = '';
Blockly.LANG_TEXT_JOIN_TITLE_CREATEWITH = "créer un texte avec";//'create text with';
Blockly.LANG_TEXT_JOIN_TOOLTIP = "Crée un texte en méttant\n" +
  "bout a bout plusieurs éléments.";

Blockly.LANG_TEXT_CREATE_JOIN_TITLE_JOIN = 'mètre bout a bout';
Blockly.LANG_TEXT_CREATE_JOIN_TOOLTIP =
  "Ajoute, supprime ou réordonne les sections\n" +
  "pour reconfigurer ce bloc de texte.";//'Add, remove, or reorder sections to reconfigure this text block.';

Blockly.LANG_TEXT_CREATE_JOIN_ITEM_TITLE_ITEM = 'élément';
Blockly.LANG_TEXT_CREATE_JOIN_ITEM_TOOLTIP = "Ajoute un élément au texte.";//'Add an item to the text.';

Blockly.LANG_TEXT_APPEND_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html'; 
Blockly.LANG_TEXT_APPEND_TO = 'dans';
Blockly.LANG_TEXT_APPEND_APPENDTEXT = 'ajoute le texte';
Blockly.LANG_TEXT_APPEND_VARIABLE = 'élément';
Blockly.LANG_TEXT_APPEND_TOOLTIP = 'Ajouter un texte a la variable "%1"';//'Append some text to variable "%1".';

Blockly.LANG_TEXT_LENGTH_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html'; 
Blockly.LANG_TEXT_LENGTH_INPUT_LENGTH = 'longueur';
Blockly.LANG_TEXT_LENGTH_TOOLTIP = "Retourne le nombre de lettres (espaces compris)\n" +
  "dans le texte donné.";

Blockly.LANG_TEXT_ISEMPTY_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_TEXT_ISEMPTY_INPUT_ISEMPTY = 'est vide';
Blockly.LANG_TEXT_ISEMPTY_TOOLTIP = "Retourne vrai si le text donné est vide.";//'Returns true if the provided text is empty.';

Blockly.LANG_TEXT_ENDSTRING_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_TEXT_ENDSTRING_INPUT = 'lettres dans le texte';
Blockly.LANG_TEXT_ENDSTRING_TOOLTIP = 'Retourne le nombre de lettre demandées au debut ou a la fin. du texte.';
Blockly.LANG_TEXT_ENDSTRING_OPERATOR_FIRST = 'premières';
Blockly.LANG_TEXT_ENDSTRING_OPERATOR_LAST = 'dernières';

Blockly.LANG_TEXT_INDEXOF_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_TEXT_INDEXOF_TITLE_FIND = 'trouve la';
Blockly.LANG_TEXT_INDEXOF_INPUT_OCCURRENCE = 'occurrence du texte';
Blockly.LANG_TEXT_INDEXOF_INPUT_INTEXT = 'dans le texte';
Blockly.LANG_TEXT_INDEXOF_TOOLTIP = "Retourne l'index de la première(ou dernière) occurrence\n" +//'Returns the index of the first/last occurrence\n' +
  "du premier texte dans le second texte.\n" +
  "Retourne 0 si le texte n'est pas trouvé.";
Blockly.LANG_TEXT_INDEXOF_OPERATOR_FIRST = 'première';
Blockly.LANG_TEXT_INDEXOF_OPERATOR_LAST = 'dernière';

Blockly.LANG_TEXT_CHARAT_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm'
Blockly.LANG_TEXT_CHARAT_INPUT_AT = 'lettre n°';
Blockly.LANG_TEXT_CHARAT_INPUT_INTEXT = 'dans le texte';
Blockly.LANG_TEXT_CHARAT_FROM_END = 'letter # from end';
Blockly.LANG_TEXT_CHARAT_FIRST = 'first letter';
Blockly.LANG_TEXT_CHARAT_LAST = 'last letter';
Blockly.LANG_TEXT_CHARAT_RANDOM = 'random letter';
Blockly.LANG_TEXT_CHARAT_INPUT_INTEXT = 'in text';
Blockly.LANG_TEXT_CHARAT_TOOLTIP = 'Returns the letter at the specified position.';

Blockly.LANG_TEXT_CHANGECASE_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_TEXT_CHANGECASE_TITLE_TO = 'en';
Blockly.LANG_TEXT_CHANGECASE_TOOLTIP = "Retourne une copie du texte en majuscule ou minuscule.";//'Return a copy of the text in a different case.';
Blockly.LANG_TEXT_CHANGECASE_OPERATOR_UPPERCASE = 'MAJUSCULE';
Blockly.LANG_TEXT_CHANGECASE_OPERATOR_LOWERCASE = 'minuscule';
Blockly.LANG_TEXT_CHANGECASE_OPERATOR_TITLECASE = 'Title';

Blockly.LANG_TEXT_TRIM_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_TEXT_TRIM_TITLE_SPACE = 'rogne les espaces à';
Blockly.LANG_TEXT_TRIM_TITLE_SIDES = '';
Blockly.LANG_TEXT_TRIM_TOOLTIP = "Retourne une copie du texte sans les espaces\n"+//'Return a copy of the text with spaces\n' +
  "au début, à la fin, ou aux deux.";//'removed from one or both ends.';
Blockly.LANG_TEXT_TRIM_TITLE_SIDES = '';
Blockly.LANG_TEXT_TRIM_TITLE_SIDE = '';
Blockly.LANG_TEXT_TRIM_OPERATOR_BOTH = 'gauche et à droite';
Blockly.LANG_TEXT_TRIM_OPERATOR_LEFT = 'gauche';
Blockly.LANG_TEXT_TRIM_OPERATOR_RIGHT = 'droite';

Blockly.LANG_TEXT_PRINT_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_TEXT_PRINT_TITLE_PRINT = 'affiche';
Blockly.LANG_TEXT_PRINT_TOOLTIP = "Affiche le texte, nombre ou autre valeur donné";//'Print the specified text, number or other value.';

Blockly.LANG_TEXT_PROMPT_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode92.html';
Blockly.LANG_TEXT_PROMPT_TITLE_PROMPT_FOR = "demande un"//'prompt for';
Blockly.LANG_TEXT_PROMPT_TITILE_WITH_MESSAGE = 'avec le message';
Blockly.LANG_TEXT_PROMPT_TOOLTIP = "Demande a l'utilisateur de saisir une valeur, avec le texte donné.";//'Prompt for user input with the specified text.';
Blockly.LANG_TEXT_PROMPT_TYPE_TEXT = 'texte';
Blockly.LANG_TEXT_PROMPT_TYPE_NUMBER = 'nombre';

// Lists Blocks.
Blockly.LANG_CATEGORY_LISTS = 'Listes';
Blockly.LANG_LISTS_CREATE_EMPTY_HELPURL = 'http://en.wikipedia.org/wiki/Linked_list#Empty_lists';
Blockly.LANG_LISTS_CREATE_EMPTY_TITLE = "Crée une liste vide";//'create empty list';
Blockly.LANG_LISTS_CREATE_EMPTY_TOOLTIP = "Retourne une liste de taille 0, ne contenant aucun éléments.";//'Returns a list, of length 0, containing no data records';

Blockly.LANG_LISTS_CREATE_WITH_INPUT_WITH = "crée une liste avec";//'create list with';
Blockly.LANG_LISTS_CREATE_WITH_TOOLTIP = "Crée une liste avec un nombre quelconque d'éléments.";//'Create a list with any number of items.';

Blockly.LANG_LISTS_CREATE_WITH_CONTAINER_TITLE_ADD = 'liste';
Blockly.LANG_LISTS_CREATE_WITH_CONTAINER_TOOLTIP = "Ajout, retire, ou réordone pour reconfigurer ce bloque liste.";//'Add, remove, or reorder sections to reconfigure this list block.';

Blockly.LANG_LISTS_CREATE_WITH_ITEM_TITLE = 'élément';
Blockly.LANG_LISTS_CREATE_WITH_ITEM_TOOLTIP = "Ajoute un élément à la liste";//'Add an item to the list.';

Blockly.LANG_LISTS_REPEAT_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_LISTS_REPEAT_INPUT_WITH = 'create list with item';
Blockly.LANG_LISTS_REPEAT_INPUT_REPEATED = 'repeated';
Blockly.LANG_LISTS_REPEAT_INPUT_TIMES = 'times';
Blockly.LANG_LISTS_REPEAT_TOOLTIP = 'Creates a list consisting of the given value\n' +
    'repeated the specified number of times.';

Blockly.LANG_LISTS_LENGTH_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_LISTS_LENGTH_INPUT_LENGTH = 'longueur';
Blockly.LANG_LISTS_LENGTH_TOOLTIP = "Retourne le nombre d'élément dans liste.";//'Returns the length of a list.';

Blockly.LANG_LISTS_IS_EMPTY_HELPURL = 'http://www.liv.ac.uk/HPC/HTMLF90Course/HTMLF90CourseNotesnode91.html';
Blockly.LANG_LISTS_INPUT_IS_EMPTY = 'est vide';
Blockly.LANG_LISTS_TOOLTIP = "Retourne 'vrai' si la liste est vide.";//'Returns true if the list is empty.';

Blockly.LANG_LISTS_INDEX_OF_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_LISTS_INDEX_OF_TITLE_FIND = 'find';
Blockly.LANG_LISTS_INDEX_OF_INPUT_OCCURRENCE = 'occurrence of item';
Blockly.LANG_LISTS_INDEX_OF_INPUT_IN_LIST = 'in list';
Blockly.LANG_LISTS_INDEX_OF_TOOLTIP = 'Returns the index of the first/last occurrence\n' +
    'of the item in the list.\n' +
    'Returns 0 if text is not found.';
Blockly.LANG_LISTS_INDEX_OF_FIRST = 'first';
Blockly.LANG_LISTS_INDEX_OF_LAST = 'last';

Blockly.LANG_LISTS_GET_INDEX_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_LISTS_GET_INDEX_INPUT_AT = 'get item at';
Blockly.LANG_LISTS_GET_INDEX_GET_REMOVE = 'get and remove';
Blockly.LANG_LISTS_GET_INDEX_REMOVE = 'remove';
Blockly.LANG_LISTS_GET_INDEX_FROM_START = '#';
Blockly.LANG_LISTS_GET_INDEX_FROM_END = '# from end';
Blockly.LANG_LISTS_GET_INDEX_FIRST = 'first';
Blockly.LANG_LISTS_GET_INDEX_LAST = 'last';
Blockly.LANG_LISTS_GET_INDEX_RANDOM = 'random';
Blockly.LANG_LISTS_GET_INDEX_INPUT_IN_LIST = 'in list';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP = 'Returns the value at the specified position in a list.';
    '#1 is the first item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_FROM_END = 'Returns the item at the specified position in a list.\n' +
    '#1 is the last item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_FIRST = 'Returns the first item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_LAST = 'Returns the last item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_RANDOM = 'Returns a random item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_REMOVE_FROM_START = 'Removes and returns the item at the specified position\n' +
    ' in a list.  #1 is the first item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_REMOVE_FROM_END = 'Removes and returns the item at the specified position\n' +
    ' in a list.  #1 is the last item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_REMOVE_FIRST = 'Removes and returns the first item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_REMOVE_LAST = 'Removes and returns the last item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_GET_REMOVE_RANDOM = 'Removes and returns a random item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_REMOVE_FROM_START = 'Removes the item at the specified position\n' +
    ' in a list.  #1 is the first item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_REMOVE_FROM_END = 'Removes the item at the specified position\n' +
    ' in a list.  #1 is the last item.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_REMOVE_FIRST = 'Removes the first item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_REMOVE_LAST = 'Removes the last item in a list.';
Blockly.LANG_LISTS_GET_INDEX_TOOLTIP_REMOVE_RANDOM = 'Removes a random item in a list.';

Blockly.LANG_LISTS_SET_INDEX_HELPURL = 'http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Farsubex.htm';
Blockly.LANG_LISTS_SET_INDEX_INPUT_AT = 'set item at';
Blockly.LANG_LISTS_SET_INDEX_INPUT_IN_LIST = 'in list';
Blockly.LANG_LISTS_SET_INDEX_INPUT_TO = 'to';
Blockly.LANG_LISTS_SET_INDEX_TOOLTIP = 'Sets the value at the specified position in a list.';

// Variables Blocks.
Blockly.LANG_VARIABLES_GET_HELPURL = 'http://en.wikipedia.org/wiki/Variable_(computer_science)';
Blockly.LANG_VARIABLES_GET_TITLE = 'avec';
Blockly.LANG_VARIABLES_GET_ITEM = 'élément';
Blockly.LANG_VARIABLES_GET_TOOLTIP = "Retourns la valeur de cette variable.";

Blockly.LANG_VARIABLES_SET_HELPURL = 'http://en.wikipedia.org/wiki/Variable_(computer_science)';
Blockly.LANG_VARIABLES_SET_TITLE = 'dans';
Blockly.LANG_VARIABLES_SET_ITEM = 'élément';
Blockly.LANG_VARIABLES_SET_TITLE_2 = 'place';
Blockly.LANG_VARIABLES_SET_TOOLTIP = "Place la valeur donnée dans la valiable";

// Procedures Blocks.
Blockly.LANG_PROCEDURES_DEFNORETURN_HELPURL = 'http://en.wikipedia.org/wiki/Procedure_%28computer_science%29';
Blockly.LANG_PROCEDURES_DEFNORETURN_PROCEDURE = 'procédure';
Blockly.LANG_PROCEDURES_DEFNORETURN_DO = 'fait';
Blockly.LANG_PROCEDURES_DEFNORETURN_TOOLTIP = "Une procédure qui ne retourne pas de valeure.";

Blockly.LANG_PROCEDURES_DEFRETURN_HELPURL = 'http://en.wikipedia.org/wiki/Procedure_%28computer_science%29';
Blockly.LANG_PROCEDURES_DEFRETURN_PROCEDURE = Blockly.LANG_PROCEDURES_DEFNORETURN_PROCEDURE;
Blockly.LANG_PROCEDURES_DEFRETURN_DO = Blockly.LANG_PROCEDURES_DEFNORETURN_DO;
Blockly.LANG_PROCEDURES_DEFRETURN_RETURN = 'retourne';
Blockly.LANG_PROCEDURES_DEFRETURN_TOOLTIP = "Une procédure qui retourne une valeur.";//'A procedure with a return value.';

Blockly.LANG_PROCEDURES_DEF_DUPLICATE_WARNING = "Attention:\n" +//'Warning:\n' +
    "Cette procédure comporte\n" +//'This procedure has\n' +
    "des paramètres en double.";//'duplicate parameters.';

Blockly.LANG_PROCEDURES_CALLNORETURN_HELPURL = 'http://en.wikipedia.org/wiki/Procedure_%28computer_science%29';
Blockly.LANG_PROCEDURES_CALLNORETURN_CALL = 'éxcécute';
Blockly.LANG_PROCEDURES_CALLNORETURN_PROCEDURE = 'la procédure';
Blockly.LANG_PROCEDURES_CALLNORETURN_TOOLTIP =
  "Excecute une procédure qui ne retourne rien.";//'Call a procedure with no return value.';

Blockly.LANG_PROCEDURES_CALLRETURN_HELPURL = 'http://en.wikipedia.org/wiki/Procedure_%28computer_science%29';
Blockly.LANG_PROCEDURES_CALLRETURN_CALL = Blockly.LANG_PROCEDURES_CALLNORETURN_CALL;
Blockly.LANG_PROCEDURES_CALLRETURN_PROCEDURE = Blockly.LANG_PROCEDURES_CALLNORETURN_PROCEDURE;
Blockly.LANG_PROCEDURES_CALLRETURN_TOOLTIP = "Excécute une procédure qui retourne une valeur.";//'Call a procedure with a return value.';

Blockly.LANG_PROCEDURES_MUTATORCONTAINER_TITLE = 'paramètres';
Blockly.LANG_PROCEDURES_MUTATORARG_TITLE = 'variable:';

Blockly.LANG_PROCEDURES_HIGHLIGHT_DEF = 'Procédure de surbrillance';

Blockly.LANG_PROCEDURES_IFRETURN_TOOLTIP = 'If a value is true, then return a value.';
Blockly.LANG_PROCEDURES_IFRETURN_WARNING = 'Warning:\n' +
    'This block may only be\n' +
    'used within a procedure.';
