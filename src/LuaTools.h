#ifndef __LUA_TOOLS__
#define __LUA_TOOLS__

#include "stdafx.h"
#include <luabind/error.hpp>

extern "C"
{
#include <lua.h>
}

namespace LuaTools
{

std::string GetLuabindErrorString(const luabind::error& err);

class LuaEngine
{
	lua_State* L;

	static int add_file_and_line(lua_State* L);

	static int  panicf(lua_State* L);

public:
	LuaEngine();

	~LuaEngine();

	lua_State* state();
};

}

#endif //__LUA_TOOLS__
