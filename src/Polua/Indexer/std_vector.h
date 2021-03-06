//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef __POLUA_INDEXER_STD_VECTOR__
#define __POLUA_INDEXER_STD_VECTOR__

#include <vector>
#include "../Class.h"

namespace Polua
{
//! Traits pour que Polua sache indexer(utiliser operateur []) les std::vector
template<typename V>
struct Indexer<std::vector<V> >
{
	//! Récupere une valeur dans le std::vector
	static int get(lua_State* L)
	{
		int isnum = 0;
		size_t const index = lua_tounsignedx(L, -1, &isnum);
		if(isnum == 0)
			return 0;
		std::vector<V>* vect = userdata_fromstack<std::vector<V> >(L, 1);
		if(index < 1 || index > vect->size())
			return 0;
		typename std::vector<V>::reference val = (*vect)[index - 1];
		Polua::pushPers(L, val);
		return 1;
	}

	//! set une valeur dans le std::vector
	static int set(lua_State* L)
	{
		int isnum = 0;
		size_t const index = lua_tounsignedx(L, -2, &isnum);
		if(isnum == 0)
			return 0;
		std::vector<V>* vect = userdata_fromstack<std::vector<V> >(L, 1);
		if(index < 1 || index > vect->size())
			return 0;
		(*vect)[index - 1] = Polua::fromstackAny<V>(L, -1);
		return 0;
	}
};


//! Taits pour que lua sache iterer un std::vector
template<typename T>
struct IPairs<std::vector<T> >
{
	typedef std::vector<T> Container; //!< std::vector<T>

	//! Comme la methode lua ipairs. Pour les std::vector
	static int ipairs(lua_State* L)
	{
		lua_pushcfunction(L, &IPairs<Container>::iterator);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}

	//! @brief Comme la methode lua ipairs. Pour les std::vector.
	//! @remarks Sur un vector, ipairs et pairs font la même chose.
	static int pairs(lua_State* L)
	{
		return ipairs(L);
	}

private:

	//! Iterateur fonctionel lua, pour les std::vector
	static int iterator(lua_State* L)
	{
		/* 1: table
		* 2: key
		*/
		Container* obj = userdata_fromstack<Container>(L, 1);
		//typedef typename Container::iterator iterator;
		size_t index =
		  (lua_gettop(L) < 2 || lua_isnil(L, 2)) ?
		  1 :
		  Polua::fromstackAny<size_t>(L, 2) + 1;

		if(index > obj->size())
		{
			lua_pushnil(L);
			return 1;
		}

		Polua::pushTemp(L, index);
		//typedef typename std::vector<T>::reference reference;
		Polua::pushPers(L, (*obj)[index - 1]);
		return 2;
	}
};

//! Traits pour que Polua puisse obtenir la taille d'un std::vector
template<typename T>
struct Length<std::vector<T> >
{
	typedef std::vector<T> Container; //!< std::vector<T>

	//! Retourne la taille du std::vector, en 1 sur la pile lua.
	static int len(lua_State* L)
	{
		Container* obj = userdata_fromstack<Container>(L, 1);
		lua_pushinteger(L, obj->size());
		return 1;
	}
};

}

#endif //__POLUA_INDEXER_STD_VECTOR__