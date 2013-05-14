//! @file
//! @author Loïc HAMOT

#ifndef __LUA_TOOLS__
#define __LUA_TOOLS__

#include "stdafx.h"
#include <luabind/error.hpp>

extern "C"
{
#include <lua.h>
}


//! Outils divers aidant l'utilisation de lua dans du c++
namespace LuaTools
{

//! Recupère le dernier message d'erreur laissé par lua
std::string GetLuabindErrorString(const luabind::error& err);

//! Réencaplusation plus RAII d'un lua_State
class LuaEngine
{
	lua_State* L; //!< Toute les données de la VM lua

	//! Ajoute le nom de fichier et numero de ligne dans la pile
	static int add_file_and_line(lua_State* L);

	//! Gère une erreur remontée par la VM lua
	static int  panicf(lua_State* L);

public:
	//! Constructeur
	LuaEngine();

	//! Déstructeur
	~LuaEngine();

	//! Renvoie le lua_State (environement lua)
	lua_State* state();
};

}

#endif //__LUA_TOOLS__
