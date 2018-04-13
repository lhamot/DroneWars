//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __BTA_LUA_UNIVERSE__
#define __BTA_LUA_UNIVERSE__

#include "LuaTools.h"

extern "C"
{
#include "lua.h"
}

//! Enseigne l'API lua de DroneWars à la VM lua
int initDroneWars(LuaTools::Engine& engine);

#endif //__BTA_LUA_UNIVERSE__