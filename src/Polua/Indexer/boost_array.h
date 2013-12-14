//! @file
//! @author Loïc HAMOT
#ifndef __POLUA_INDEXER_BOOST_ARRAY__
#define __POLUA_INDEXER_BOOST_ARRAY__

#include <boost/array.hpp>
#include "../Class.h"

namespace Polua
{
//! Traits pour que Polua sache indexer(utiliser operateur []) les boost::array
template<typename V, size_t S>
struct Indexer<boost::array<V, S> >
{
	typedef boost::array<V, S> Container; //!< boost::array<V, S>

	//! lua_Cfunction equivalent a un .at(index) sur un boost::array
	static int get(lua_State* L)
	{
		POLUA_CHECK_STACK(L, 1);
		int isnum = 0;
		size_t const index = lua_tounsignedx(L, -1, &isnum);
		if(isnum == 0)
			return 0;
		Container* vect = userdata_fromstack<Container>(L, 1);
		if(index < 1 || index > vect->size())
			return 0;
		typename Container::reference val = (*vect)[index - 1];
		Polua::pushPers(L, val);
		return 1;
	}

	//! lua_Cfunction equivalent a un .at(index) = V sur un boost::array
	static int set(lua_State* L)
	{
		POLUA_CHECK_STACK(L, 0);
		int isnum = 0;
		size_t const index = lua_tounsignedx(L, -2, &isnum);
		if(isnum == 0)
			return 0;
		Container* vect = userdata_fromstack<Container>(L, 1);
		if(index < 1 || index > vect->size())
			return 0;
		(*vect)[index - 1] = Polua::fromstackAny<V>(L, -1);
		return 0;
	}
};


//! Traits pour que Polua sache iterer les boost::array
template<typename V, size_t S>
struct IPairs<boost::array<V, S> >
{
	typedef boost::array<V, S> Container; //!< boost::array<V, S>

	//! Comme la methode lua ipairs. Pour les std::vector
	static int ipairs(lua_State* L)
	{
		Container* obj = userdata_fromstack<Container>(L, -1);
		if(!obj)
			return luaL_error(L , "Internal error, no object given!");

		lua_pushcfunction(L, &IPairs<Container>::iterator);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}

	//! @brief Comme la methode lua ipairs. Pour les boost::array.
	//! @remarks Sur un array, ipairs et pairs font la même chose.
	static int pairs(lua_State* L)
	{
		return ipairs(L);
	}

private:
	//! Iterateur fonctionel lua, pour les boost::array
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
		  index = Polua::fromstackAny<size_t>(L, 2) + 1;

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

}

#endif //__POLUA_INDEXER_BOOST_ARRAY__