#include "TypedPtree.h"
#include "Polua/Class.h"
#include "Tools.h"

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
//! @return 1
int ptree_put(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::add
//! @return 1
int ptree_add(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::add_child
//! @return 1
int ptree_add_child(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::put_child
//! @return 1
int ptree_put_child(lua_State* L);

//! @brief lua_CFunction pour la function TypePtree::put_value
//! @return 0
int ptree_put_value(lua_State* L);

TypedPtree& ptree_get_child_(TypedPtree& pt, std::string const& name);

//! @brief lua_CFunction pour la function TypePtree::get_child_optional
//! @remark Contrairement a TypePtree::get_child_optional,
//!         Quand le child n'existe pas, il est crée.
//! @return 1
int ptree_get_child(lua_State* L);

//! lua_CFunction qui Supprime tout les enfants direct qui ont la clef donné,
//!   et retourne le nombre
//! @return 1
int ptree_erase(lua_State* L);


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
		return putter(memory, Any());
		break;
	case LUA_TNUMBER:
		return putter(memory, Any(lua_tonumber(L, -1)));
		break;
	case LUA_TBOOLEAN:
		return putter(memory, Any(lua_toboolean(L, -1) != 0));
		break;
	case LUA_TSTRING:
		return putter(memory, Any(std::string(lua_tostring(L, -1))));
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
	BOOST_THROW_EXCEPTION(std::logic_error("Bad lua_type"));
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
			pushPers(L, ptree_get_child_(*pt, LEXICAL_CAST(std::string, path)));
			return 1;
		}
		case LUA_TBOOLEAN:
		{
			int const path = lua_toboolean(L, -1);
			pushPers(L, ptree_get_child_(*pt, LEXICAL_CAST(std::string, path)));
			return 1;
		}
		case LUA_TSTRING:
			pushPers(L, ptree_get_child_(*pt, lua_tostring(L, -1)));
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
			return set(L, LEXICAL_CAST(std::string, path));
		}
		case LUA_TBOOLEAN:
		{
			int const path = lua_toboolean(L, -2);
			return set(L, LEXICAL_CAST(std::string, path));
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
			Polua::pushPers(L, pt.put(key, value));
			return 1;
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

		auto pushValue = [&](TypedPtree::iterator & iter)
		{
			if(iter == obj->end())
			{
				lua_pushnil(L);
				return 1;
			}
			else
			{
				Polua::pushPers(L, iter->second);
				return 2;
			}
		};

		if(lua_gettop(L) < 2 || lua_isnil(L, 2))
		{
			TypedPtree::iterator iter = obj->begin();
			Polua::pushTemp(L, iter);
			return pushValue(iter);
		}
		else
		{
			TypedPtree::iterator& iter = *userdata_fromstack<TypedPtree::iterator>(L, 2);
			++iter;
			lua_pushvalue(L, 2);
			return pushValue(iter);
		}
	}
};

//! Traits pour que Polua puisse obtenir la taille d'un std::vector
template<>
struct Length<TypedPtree>
{
	typedef TypedPtree Container;

	//! Retourne la taille du std::vector, en 1 sur la pile lua.
	static int len(lua_State* L)
	{
		Container* obj = userdata_fromstack<Container>(L, 1);
		lua_pushinteger(L, obj->size());
		return 1;
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