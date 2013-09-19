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


//! Nombre d'instruction max authorisé dans l'éxcecution d'un script lua
static size_t const LuaMaxInstruction = 20000;


//! callback appelé par lua quand le nombre d'instruction max est dépassé
void luaCountHook(lua_State* L, lua_Debug* ar);

}

#endif //__LUA_TOOLS__
