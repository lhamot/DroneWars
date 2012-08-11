// Copyright (c) 2010 Thomas Nelson

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#include "stdafx.h"

#define LUABIND_BUILDING

#include <luabind/config.hpp>
#include <luabind/lua_include.hpp>
#include <luabind/detail/stl_container_adapter.hpp>

namespace luabind
{
namespace detail
{
static int s_stl_weak_registry = LUA_REFNIL;

static void stl_weak_registry_create(lua_State* L)
{
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "__mode");
	lua_pushstring(L, "v");
	lua_settable(L, -3);
	lua_setmetatable(L, -2);

	s_stl_weak_registry = luaL_ref(L, LUA_REGISTRYINDEX);
}

// table should be at top of stack
void stl_register_collection(lua_State* L, void* p)
{

	if(s_stl_weak_registry == LUA_REFNIL)
	{
		stl_weak_registry_create(L);
	}
	lua_pushinteger(L, s_stl_weak_registry);
	lua_gettable(L, LUA_REGISTRYINDEX);
	// registry now at top of stack
	lua_pushlightuserdata(L, p);
	lua_pushvalue(L, -3);	// copy the table
	lua_settable(L, -3);
	lua_pop(L, 1);
}

bool stl_find_table_for_collection(lua_State* L, void* p)
{
	lua_pushinteger(L, s_stl_weak_registry);
	lua_gettable(L, LUA_REGISTRYINDEX);

	if(lua_isnil(L, -1))
	{
		// not found
		lua_pop(L, 1);
		return false;
	}
	lua_pushlightuserdata(L, p);
	lua_gettable(L, -2);

	if(lua_isnil(L, -1))
	{
		lua_pop(L, 2);
		return false;
	}
	lua_remove(L, -2);
	return true;
}

int stl_container__newindex_readonly_impl(lua_State* L)
{
	/* 1: table object
	 * 2: key
	 * 3: value
	 */
	return luaL_error(L, "Attempt to modify a read only table.");
}

}
}
