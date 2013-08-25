//! @file
//! @author Loïc HAMOT

#ifndef __LUA_TOOLS__
#define __LUA_TOOLS__

#include "stdafx.h"
#include "Polua/Core.h"
#include "Polua/State.h"

extern "C"
{
#include <lua.h>
}


//! Outils divers aidant l'utilisation de lua dans du c++
namespace LuaTools
{
typedef Polua::State LuaEngine; //!< Interpréteur lua
}

#endif //__LUA_TOOLS__
