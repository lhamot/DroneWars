//! @file
//! @author Loïc HAMOT

#ifndef __LUA_TOOLS__
#define __LUA_TOOLS__

#include "stdafx.h"
#include "Polua/Core.h"
#include "Polua/State.h"
#include "Polua/Ref.h"

extern "C"
{
#include <lua.h>
}


//! Outils divers aidant l'utilisation de lua dans du c++
namespace LuaTools
{
typedef Polua::object Object;
typedef Polua::State Engine; //!< Interpréteur lua


//! Nombre d'instruction max authorisé dans l'éxcecution d'un script lua
static size_t const LuaMaxInstruction = 20000;


//! callback appelé par lua quand le nombre d'instruction max est dépassé
void luaCountHook(lua_State* L, lua_Debug* ar);

//! Ce Polua::object est t'il une fonction
inline bool isFunction(Polua::object const& obj)
{
	return (obj && obj->is_valid() && obj->type() == LUA_TFUNCTION);
}

inline Object refFromName(Engine& engine, std::string const& name)
{
	return Polua::refFromName(engine.state(), name);
}

inline bool isValid(Object const& obj)
{
	return obj->is_valid();
}

template<typename T>
inline T extract(Object const& obj)
{
	return obj->get<T>();
}

}

#endif //__LUA_TOOLS__
