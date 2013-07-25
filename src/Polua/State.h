//! @file
//! @author Loïc HAMOT
#ifndef __POLUA_STATE__
#define __POLUA_STATE__

#include <string>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace Polua
{
//! Contient un lua_State et gère ca suppresion automatiquement
class State
{
	State(State const&);
	State& operator=(State const&);

	lua_State* const state_;

public:
	State(): state_(luaL_newstate())
	{
		if(state_ == NULL)
			throw std::bad_alloc();
	}

	~State()
	{
		lua_close(state_);
	}

	void openlibs()
	{
		luaL_openlibs(state_);
	}

	int loadString(std::string const& code,
	               std::string const& tag = std::string())
	{
		return luaL_loadbuffer(
		         state_, code.c_str(), code.size(), tag.c_str());
	}

	lua_State* state()
	{
		return state_;
	}
};
}

#endif //__POLUA_STATE__