#include "TypedPtree.h"
#include "Polua/Class.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

//! @brief lua_CFunction pour la function TypePtree::get_optional
//! @return 1
int ptree_get(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::get_value_optional
//! @return 1
int ptree_get_value(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::put
//! @return 0
int ptree_put(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::add
//! @return 0
int ptree_add(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::put_value
//! @return 0
int ptree_put_value(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::get_child_optional
//! @remark Contrairement a TypePtree::get_child_optional,
//!         Quand le child n'existe pas, il est crée.
//! @return 1
TypedPtree& ptree_get_child(TypedPtree& pt, std::string const& name);

//! Ajoute une valeur dans un TypedPtree en utilisant le Putter donné
//! @remark Un lua_CFunction peut utiliser cette fonction
//!         et retourner son resultat
//! @return 0 ou luaL_error
template<typename Putter>
int addValToPtree(lua_State* L, TypedPtree& memory, Putter putter)
{
	int const valuetype = lua_type(L, -1);
	switch(valuetype)
	{
	case LUA_TNIL:
		putter(memory, Any());
		break;
	case LUA_TNUMBER:
		putter(memory, Any(lua_tonumber(L, -1)));
		break;
	case LUA_TBOOLEAN:
		putter(memory, Any(lua_toboolean(L, -1) != 0));
		break;
	case LUA_TSTRING:
		putter(memory, Any(std::string(lua_tostring(L, -1))));
		break;
	case LUA_TTABLE:
	case LUA_TFUNCTION:
	case LUA_TUSERDATA:
	case LUA_TTHREAD:
	case LUA_TLIGHTUSERDATA:
		return luaL_error(L,
		                  "Trying to put a %s in a memory. It is impossible.",
		                  lua_typename(L, valuetype));
		break;
	}
	return 0;
}

namespace Polua
{
//! Traits pour que Polua sache indexer(utiliser operateur []) les TypedPtree
template<>
struct Indexer<TypedPtree>
{
	//! lua_CFunction pour du code comme : v = ptree["path"]
	static int get(lua_State* L)
	{
		using namespace boost;
		TypedPtree* pt = userdata_fromstack<TypedPtree>(L, 1);
		int const valuetype = lua_type(L, -1);
		switch(valuetype)
		{
		case LUA_TNUMBER:
		{
			double const path = lua_tonumber(L, -1);
			pushPers(L, ptree_get_child(*pt, lexical_cast<std::string>(path)));
			return 1;
		}
		case LUA_TBOOLEAN:
		{
			int const path = lua_toboolean(L, -1);
			pushPers(L, ptree_get_child(*pt, lexical_cast<std::string>(path)));
			return 1;
		}
		case LUA_TSTRING:
			pushPers(L, ptree_get_child(*pt, lua_tostring(L, -1)));
			return 1;
		default:
			return luaL_error(L,
			                  "Trying to use a %s ad key.",
			                  lua_typename(L, valuetype));
		}
	}

	//! lua_CFunction pour du code comme : ptree["path"] = v
	static int set(lua_State* L)
	{
		int const valuetype = lua_type(L, -2);
		switch(valuetype)
		{
		case LUA_TNUMBER:
		{
			double const path = lua_tonumber(L, -2);
			return set(L, boost::lexical_cast<std::string>(path));
		}
		case LUA_TBOOLEAN:
		{
			int const path = lua_toboolean(L, -2);
			return set(L, boost::lexical_cast<std::string>(path));
		}
		case LUA_TSTRING:
			return set(L, lua_tostring(L, -2));
		default:
			return luaL_error(L,
			                  "Trying to use a %s ad key.",
			                  lua_typename(L, valuetype));
		}
	}

private:
	//! Recupere le TypedPtree en 1 sur la pile et lui set la valeur en -1
	static int set(lua_State* L, std::string const& key)
	{
		TypedPtree* pt = userdata_fromstack<TypedPtree>(L, 1);
		return addValToPtree(L, *pt, [&](TypedPtree & pt, Any const & value)
		{
			pt.put(key, value);
		});
	}
};


//! @brief Traits pour que Polua sache itterrer un TypedPtree avec pairs
//! @remark Ca ne marche pas avec ipairs
//! @see http://lua-users.org/wiki/GeneralizedPairsAndIpairs
template<>
struct IPairs<TypedPtree>
{
	//! @brief lua_CFunction : métamethode __ipairs pour TypedPtree
	//! @warning Ce code n'est pas sensé fonctioner et déclenche une erreur lua
	static int ipairs(lua_State* L)
	{
		return luaL_error(L , "Can't iterate by index a TypedPtree");
	}

	//! @brief lua_CFunction : métamethode __pairs pour TypedPtree
	static int pairs(lua_State* L)
	{
		lua_pushcfunction(L, &IPairs<TypedPtree>::iterator);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}

private:
	//! @brief lua_CFunction : fonction next pour TypedPtree
	static int iterator(lua_State* L)
	{
		TypedPtree* obj = userdata_fromstack<TypedPtree>(L, 1);
		TypedPtree::iterator iter;
		if(lua_gettop(L) < 2 || lua_isnil(L, 2))
			iter = obj->begin();
		else
		{
			iter = *userdata_fromstack<TypedPtree::iterator>(L, 2);
			++iter;
		}

		if(iter == obj->end())
		{
			lua_pushnil(L);
			return 1;
		}

		Polua::pushTemp(L, iter);
		Polua::pushPers(L, iter->second);
		return 2;
	}
};

}

//! @brief lua_CFunction qui convertie un TypedPtree en string (pour print)
int ptree_tostring(lua_State* L);

//! @brief Donne le nom d'un élément pointé pas un TypedPtree::iterator
TypedPtree::iterator::value_type::first_type
ptree_iter_key(TypedPtree::iterator const& iter);

//! lua_CFunction qui convertie un TypedPtree::iterator en string (pour print)
int ptree_iter_tostring(lua_State* L);