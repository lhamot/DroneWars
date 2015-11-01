#include "stdafx.h"
#include "PTreeLuaHelper.h"
#include "Tools.h"

void push(lua_State* L, Any const& any)
{
	switch((AnyTypeList)any.which())
	{
	case AnyTypeList::Nil:
		lua_pushnil(L);
		break;
	case AnyTypeList::Boolean:
		lua_pushboolean(L, boost::get<bool>(any));
		break;
	case AnyTypeList::Integer:
		lua_pushinteger(L, boost::get<ptrdiff_t>(any));
		break;
	case AnyTypeList::Decimal:
		lua_pushnumber(L, boost::get<double>(any));
		break;
	case AnyTypeList::Text:
		Polua::pushPers(L, boost::get<std::string>(any));
		break;
	}
}

TypedPtree& getThisPTree(lua_State* L)
{
	TypedPtree* memory = Polua::userdata_fromstack<TypedPtree>(L, 1);
	if(!memory)
	{
		luaL_error(L, "Not a %s on the stack, as expected", typeid(TypedPtree).name());
		abort();
	}
	return *memory;
}

int ptree_get(lua_State* L)
{
	TypedPtree const& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 1)
		return luaL_error(L,
		                  "memory.get() expect one argument but got %d", nbarg);
	int const keytype = lua_type(L, -1);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.get() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const name = lua_tostring(L, -1);
	boost::optional<TypedPtree const&> childOpt = memory.get_child_optional(name);
	if(childOpt == boost::none)
		lua_pushnil(L);
	else
		push(L, childOpt->data());
	return 1;
}


int ptree_get_value(lua_State* L)
{
	TypedPtree const& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 0)
		return luaL_error(L,
		                  "memory.get_value() expect no argument but got %d",
		                  nbarg);
	push(L, memory.data());
	return 1;
}

int ptree_get_child(lua_State* L)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 1)
		return luaL_error(L,
		                  "memory.get_child() expect one argument but got %d", nbarg);
	int const keytype = lua_type(L, -1);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.get_child() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const name = lua_tostring(L, -1);
	boost::optional<TypedPtree&> childOpt = memory.get_child_optional(name);
	if(childOpt == boost::none)
		lua_pushnil(L);
	else
		Polua::pushPers(L, *childOpt);
	return 1;
}

int ptree_erase(lua_State* L)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 1)
		return luaL_error(L,
		                  "memory.get_child() expect one argument but got %d", nbarg);
	int const keytype = lua_type(L, -1);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.get_child() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const name = lua_tostring(L, -1);
	Polua::pushTemp(L, memory.erase(name));
	return 1;
}


template<typename Putter>
int ptree_putadd(lua_State* L, Putter putter)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 2)
		return luaL_error(
		         L, "memory.put/add() expect two argument but got %d", nbarg);
	int const keytype = lua_type(L, -2);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.get() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const path = lua_tostring(L, -2);
	return addValToPtree(L, memory, [&]
	                     (TypedPtree & memory, Any const & value)
	{
		putter(memory, path, value);
		return 1;
	});
}


int ptree_put(lua_State* L)
{
	return ptree_putadd(L, [&]
	                    (TypedPtree & pt, std::string const & name, Any const & val)
	{
		Polua::pushPers(L, pt.put(name, val));
	});
}

int ptree_add(lua_State* L)
{
	return ptree_putadd(L, [&]
	                    (TypedPtree & pt, std::string const & name, Any const & val)
	{
		Polua::pushPers(L, pt.add(name, val));
	});
}

int ptree_add_child(lua_State* L)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 2)
		return luaL_error(
		         L, "memory.add_child() expect two argument but got %d", nbarg);
	int const keytype = lua_type(L, -2);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.add_child() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const path = lua_tostring(L, -2);
	TypedPtree const& child = Polua::fromstackAny<TypedPtree>(L, -1);
	Polua::pushPers(L, memory.add_child(path, child));
	return 1;
}

int ptree_put_child(lua_State* L)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 2)
		return luaL_error(
		         L, "memory.put_child() expect two argument but got %d", nbarg);
	int const keytype = lua_type(L, -2);
	if(keytype != LUA_TSTRING)
		return luaL_error(L,
		                  "memory.put_child() expect a string but got a %s",
		                  lua_typename(L, keytype));
	std::string const path = lua_tostring(L, -2);
	TypedPtree const& child = Polua::fromstackAny<TypedPtree>(L, -1);
	Polua::pushPers(L, memory.put_child(path, child));
	return 1;
}

int ptree_put_value(lua_State* L)
{
	TypedPtree& memory = getThisPTree(L);
	int const nbarg = lua_gettop(L) - 1;
	if(nbarg != 1)
		return luaL_error(
		         L, "memory.put_value() expect one argument but got %d", nbarg);
	return addValToPtree(L, memory, [&]
	                     (TypedPtree & pt, Any const & value)
	{
		pt.put_value(value);
		return 0;
	});
}


TypedPtree& ptree_get_child_(TypedPtree& pt, std::string const& name)
{
	boost::optional<TypedPtree&> opt = pt.get_child_optional(name);
	if(opt)
		return *opt;
	else
		return pt.add_child(name, TypedPtree());
}


TypedPtree::iterator::value_type::first_type
ptree_iter_key(TypedPtree::iterator const& iter)
{
	return iter->first;
}


int ptree_iter_tostring(lua_State* L)
{
	TypedPtree::iterator const& iter =
	  Polua::fromstackAny<TypedPtree::iterator>(L, 1);
	std::string const& path = iter->first;
	lua_pushstring(L, path.data());
	return 1;
}

int ptree_tostring(lua_State* L)
{
	using namespace boost;
	TypedPtree const& pt = getThisPTree(L);
	optional<Any> const& value = pt.get_value_optional<Any>();
	if(value)
	{
		switch((AnyTypeList)value->which())
		{
		case AnyTypeList::Nil:
			lua_pushnil(L);
			return 1;
		case AnyTypeList::Boolean:
			Polua::pushTemp(L, LEXICAL_CAST(std::string, get<bool>(*value)));
			return 1;
		case AnyTypeList::Integer:
			Polua::pushTemp(L, LEXICAL_CAST(std::string, get<int64_t>(*value)));
			return 1;
		case AnyTypeList::Decimal:
			Polua::pushTemp(L, LEXICAL_CAST(std::string, get<double>(*value)));
			return 1;
		case AnyTypeList::Text:
			Polua::pushTemp(L, get<std::string>(*value));
			return 1;
		}
		return 0;
	}
	else
	{
		lua_pushnil(L);
		return 1;
	}
}