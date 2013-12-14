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
	State(State const&);            //!< class non copiable
	State& operator=(State const&); //!< class non copiable

	lua_State* const state_; //!< Etats lua internes

	//! Appelée par lua pour gerer une erreur quand on est PAS dans un call
	static int at_panic(lua_State* L)
	{
		char const* mess = lua_tostring(L, -1);
		std::string message = mess ? mess : "lua error without message";
		lua_pop(L, 1);
		throw Polua::Exception(0, message);
	}

public:
	//! ctor
	State(): state_(luaL_newstate())
	{
		if(state_ == NULL)
			throw std::bad_alloc();

		lua_atpanic(state_, &State::at_panic);
	}

	//! destructeur
	~State()
	{
		lua_close(state_);
	}

	//! Equivalent a luaL_openlibs
	void openlibs()
	{
		luaL_openlibs(state_);
	}

	//! Equivalent à luaL_loadbuffer
	int loadString(std::string const& code,
	               std::string const& tag = std::string())
	{
		return luaL_loadbuffer(
		         state_, code.c_str(), code.size(), tag.c_str());
	}

	//! Recupere le lua_State
	lua_State* state()
	{
		return state_;
	}
};
}

#endif //__POLUA_STATE__