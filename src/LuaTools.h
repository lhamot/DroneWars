//! @file
//! @author Lo�c HAMOT

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
typedef Polua::State LuaEngine; //!< Interpr�teur lua


//! Nombre d'instruction max authoris� dans l'�xcecution d'un script lua
static size_t const LuaMaxInstruction = 20000;


//! callback appel� par lua quand le nombre d'instruction max est d�pass�
void luaCountHook(lua_State* L, lua_Debug* ar);

}

#endif //__LUA_TOOLS__
