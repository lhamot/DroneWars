//! @file
//! @author Lo�c HAMOT

#ifndef __BTA_LUA_UNIVERSE__
#define __BTA_LUA_UNIVERSE__

#include "LuaTools.h"

extern "C"
{
#include "lua.h"
}

//! Enseigne l'API lua de DroneWars � la VM lua
int initDroneWars(LuaTools::Engine& engine);

#endif //__BTA_LUA_UNIVERSE__