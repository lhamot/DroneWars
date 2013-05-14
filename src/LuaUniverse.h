//! @file
//! @author Loïc HAMOT

#ifndef __BTA_LUA_UNIVERSE__
#define __BTA_LUA_UNIVERSE__

#include "stdafx.h"

extern "C"
{
#include "lua.h"

	//! Enseigne l'API lua de DroneWars à la VM lua
	extern "C" int initDroneWars(lua_State* L);
}

#endif //__BTA_LUA_UNIVERSE__