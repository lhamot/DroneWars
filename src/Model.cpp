#include "stdafx.h"
#include "Model.h"

Building const Building::List[] =
{
	{ RessourceSet(100, 0, 0), 1.6 }, //CommandCenter
	{ RessourceSet(100, 0, 0), 1.6 }, //MetalMine
	{ RessourceSet(100, 0, 0), 1.6 }, //CarbonMine
	{ RessourceSet(100, 0, 0), 1.6 }, //CristalMine
	{ RessourceSet(100, 0, 0), 1.6 }, //Factory
	{ RessourceSet(100, 0, 0), 1.6 }, //Laboratory
	{ RessourceSet(100, 0, 0), 1.6 }, //CarbonicCentral
	{ RessourceSet(100, 0, 0), 1.6 }, //SolarCentral
	{ RessourceSet(100, 0, 0), 1.6 }  //GeothermicCentral
};
static_assert(sizeof(Building::List) == (sizeof(Building) * Building::Count),
	"Building info missing");


Ship const Ship::List[] =
{
	//price               , life,  power, shield}
	{ RessourceSet(100, 0, 0),    1,       1,  1 }, //Mosquito
	{ RessourceSet(400, 0, 0),    2,       2,  2 }, //Hornet
	{ RessourceSet(2000, 0, 0),   4,       4,  4 }, //Vulture
	{ RessourceSet(10000, 0, 0),  8,       8,  8 }, //Dragon
	{ RessourceSet(40000, 0, 0),  16,     16, 16 }, //Behemoth
	{ RessourceSet(200000, 0, 0), 32,     32, 32 }, //Azathoth
	{ RessourceSet(2000, 0, 0),   4,       2,  2 }, //Queen
	{ RessourceSet(400, 0, 0),    2,       0,  2 }, //Cargo
	{ RessourceSet(2000, 0, 0),   8,       0,  4 }  //LargeCargo
};
static_assert(sizeof(Ship::List) == (sizeof(Ship) * Ship::Count),
	"Ship info missing");

Cannon const Cannon::List[] =
{
	{ RessourceSet(100, 0, 0),    1,       1,  1 },
	{ RessourceSet(400, 0, 0),    2,       2,  2 },
	{ RessourceSet(2000, 0, 0),   4,       4,  4 },
	{ RessourceSet(10000, 0, 0),  8,       8,  8 },
	{ RessourceSet(40000, 0, 0),  16,     16, 16 },
	{ RessourceSet(200000, 0, 0), 32,     32, 32 },
};
static_assert(sizeof(Cannon::List) == (sizeof(Cannon) * Cannon::Count),
	"Cannon info missing");
