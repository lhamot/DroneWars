//! @file
//! @author Lo�c HAMOT

#include "stdafx.h"
#include "LuaTools.h"



namespace LuaTools
{

//! callback appel� par lua quand le nombre d'instruction max est d�pass�
void luaCountHook(lua_State* L,
                  lua_Debug* ar
                 )
{
	lua_sethook(L, luaCountHook, LUA_MASKCOUNT, 1);
	if(lua_getinfo(L, "l", ar) == 0)
		luaL_error(L, "timeout was reached and lua_getinfo failed");
	luaL_error(L, "timeout was reached at line %d", ar->currentline);
}

}
