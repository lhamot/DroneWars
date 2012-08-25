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

#ifndef LUABIND_STL_CONTAINER_ADAPTER_HPP_INCLUDED
#define LUABIND_STL_CONTAINER_ADAPTER_HPP_INCLUDED

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

namespace luabind
{
namespace detail
{
static const char* container_lud_tag = "__luabind_stl_lud";
static const char* container_ref_tag = "__luabind_stl_ref";

//-------------------------------------------
template< typename Container_tt >
struct stl_container_keyvalue
{
	typedef unsigned int key_type;
	typedef typename Container_tt::value_type value_type;
};

template< typename Val_tt >
struct stl_container_keyvalue< std::set< Val_tt > >
{
	typedef unsigned int	key_type;
	typedef Val_tt 			value_type;
};

template< typename Key_tt, typename Val_tt >
struct stl_container_keyvalue< std::map< Key_tt, Val_tt > >
{
	typedef Key_tt	key_type;
	typedef Val_tt	value_type;
};

//----------------------------------------
template <class Container_tt>
struct stl_container__len;

template <class Container_tt>
struct stl_container__index;

// 	template <class Container_tt>
// 	struct stl_container__newindex;

template <class Container_tt>
struct stl_container__next;

template <class Container_tt >
struct stl_container__ipairs;

//----------------------------------------
void stl_register_collection(lua_State* L, void* p);
bool stl_find_table_for_collection(lua_State* L, void* p);

int stl_container__newindex_readonly_impl(lua_State* L);

//----------------------------------------
template <class Container_tt>
struct stl_adapter
{
	typedef Container_tt container_t;

	static int container_create_metatable(lua_State* L)
	{
		lua_newtable(L);

		lua_pushstring(L, "__len");
		lua_pushcclosure(L, &stl_container__len<Container_tt>::impl, 0);
		lua_rawset(L, -3);
		lua_pushstring(L, "__index");
		lua_pushcclosure(L, &stl_container__index<Container_tt>::impl, 0);
		lua_rawset(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcclosure(L, &stl_container__newindex_readonly_impl, 0);
		//			lua_pushcclosure(L, &stl_container__newindex<Container_tt>::impl, 0);
		lua_rawset(L, -3);
		lua_pushstring(L, "__next");
		lua_pushcclosure(L, &stl_container__next<Container_tt>::impl, 0);
		lua_rawset(L, -3);
		lua_pushstring(L, "__ipairs");
		lua_pushcclosure(L, &stl_container__ipairs<Container_tt>::impl, 0);
		lua_rawset(L, -3);

		ref_stl_adapter = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref_stl_adapter);

		return ref_stl_adapter;
	}

	static void container_push_lua_table(lua_State* L, container_t* pContainer)
	{
		int nOldTable;

		lua_newtable(L);
		/* *TSN*: Should I clone the metatable rather than setting this state data here? */
		lua_pushstring(L, container_lud_tag);
		lua_pushlightuserdata(L, pContainer);
		lua_rawset(L, -3);

		nOldTable = lua_getmetatable(L, -1);

		if(ref_stl_adapter == LUA_REFNIL)
			container_create_metatable(L);
		else
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref_stl_adapter);

		if(nOldTable)
		{
			lua_insert(L, -2); // swap the tables
			lua_setmetatable(L, -2);
		}

		lua_setmetatable(L, -2);

		/* *TSN*: Should I clone the metatable rather than setting this state data here? */
		lua_pushstring(L, container_ref_tag);
		lua_pushinteger(L, ref_stl_adapter);
		lua_rawset(L, -3);
	}

	static container_t* container_from_table(lua_State* L, int index)
	{
		container_t* pcontainer;

		if(!lua_istable(L, index))
			return 0;

		lua_pushstring(L, container_lud_tag);
		lua_rawget(L, index);
		pcontainer = static_cast<container_t*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return pcontainer;
	}

	static int container_match(lua_State* L, int index)
	{
		if(!lua_istable(L, index))
			return -1;	// not a table at all

		// must be a table AND must be a compatible STL table
		lua_pushstring(L, detail::container_ref_tag);
		lua_rawget(L, index);
		if(lua_isnil(L, -1))
		{
			// not one of our tables at all
			lua_pop(L, 1);
			return -1;
		}
		int nRefId = lua_tointeger(L, -1);
		lua_pop(L, 1);

		return (nRefId == ref_stl_adapter) ? 0 : -1;
	}

	static container_t* container_from(lua_State* L, int index)
	{
		lua_pushstring(L, detail::container_lud_tag);
		lua_rawget(L, index);

		container_t* raw_ptr = static_cast<container_t*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return raw_ptr;
	}

	static void container_to(lua_State* L, container_t* p)
	{
		if(!stl_find_table_for_collection(L, p))
		{
			container_push_lua_table(L, p);
			stl_register_collection(L, p);
		}
		// table now at top of stack
	}

	static int ref_stl_adapter;

};

template <class Container_tt>
int stl_adapter<Container_tt>::ref_stl_adapter = LUA_REFNIL;



/**
 * Table length operation. '#table'
 * metamethod
 * ---
 * @Note technically this is incorrect it returns the number of items in
 * the STL collection, however Lua (2.2.5) defines the length of a table
 * as follows:
 * "The length of a table t is defined to be any integer index n such
 * that t[n] is not nil and t[n+1] is nil."
 *
 * But do I want to scan looking for a nil value?
 */
template <class Container_tt >
struct stl_container__len
{
	static int impl(lua_State* L)
	{
		/* stack:
		 * 1: table object
		 */
		Container_tt* container = stl_adapter<Container_tt>::container_from_table(L, 1);

		size_t iSize(0);
		if(!container->empty())
			iSize = container->size();

		lua_pushnumber(L, double(iSize));
		return 1;
	}
};

template <class Container_tt >
struct stl_container__ipairs
{
	/*static int next(lua_State *L)
	{
		return 0;
	}*/

	static int impl(lua_State* L)
	{
		/* stack:
		 * 1: table object
		 */
		Container_tt* container = stl_adapter<Container_tt>::container_from_table(L, 1);

		lua_pushcfunction(L, &stl_container__next<Container_tt>::impl);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		return 3;
	}
};

/**
 * metatable method, accessors. val = table[key];
 * Note that lua indexes are 1 based.
 * should work for std::vector and std::deque
 */
template <class Container_tt >
struct stl_container__index
{
	typedef typename stl_container_keyvalue<Container_tt>::key_type key_type;
	typedef typename stl_container_keyvalue<Container_tt>::value_type value_type;
	typedef Container_tt container_type;

	static int impl(lua_State* L)
	{
		/* 1: table
		 * 2: key
		 */
		default_converter< value_type > valConv;
		default_converter< key_type >	keyConv;
		Container_tt* container = stl_adapter<Container_tt>::container_from_table(L, 1);
		key_type index = keyConv.apply(L, LUABIND_DECORATE_TYPE(key_type), 2);

		if((index < 1) || (index > container->size()))
		{
			// the key is not convertible to an integer or is outside our range
			lua_pushnil(L);
			return 1;
		}

		valConv.apply(L, container->at(index - 1));

		return 1;
	}
};

// specialization for maps.  This uses an arbitrary type for the index key
template <class KeyType_tt, class ValType_tt >
struct stl_container__index< std::map<KeyType_tt, ValType_tt> >
{
	typedef KeyType_tt	key_type;
	typedef ValType_tt value_type;
	typedef std::map<key_type, value_type> container_type;

	static int impl(lua_State* L)
	{
		default_converter< value_type >	valConv;
		default_converter< key_type >	keyConv;
		container_type* container = stl_adapter<container_type>::container_from_table(L, 1);
		KeyType_tt index = keyConv.apply(L, LUABIND_DECORATE_TYPE(key_type), 2);
		typename container_type::iterator it;

		it = container->find(index);
		if(it == container->end())
		{
			lua_pushnil(L);
			return 1;
		}

		valConv.apply(L, (*it).second);

		return 1;
	}
};

// specializations for lists, sets and multi-sets
// Lists do not support random access and in sets and multisets the value
// itself is the key.  For these collections get the first iterator and
// increment it to the nth item in the collection.
// (*TSN* I wonder if there is a way to do this without a macro... I don't
// think so.)
#	define LUABIND_STL_CONTAINER__INDEX(type)								\
	template <class ValType_tt>												\
	struct stl_container__index< type <ValType_tt> >						\
	{																		\
		typedef typename stl_container_keyvalue< type <ValType_tt> >::key_type key_type; \
		typedef typename stl_container_keyvalue< type <ValType_tt> >::value_type value_type; \
		typedef type <value_type> container_type;							\
		\
		static int impl(lua_State *L)										\
		{																	\
			default_converter< value_type > valConv;						\
			default_converter< key_type >	keyConv;						\
			container_type *	container = stl_adapter< container_type >::container_from_table(L, 1); \
			key_type index = keyConv.apply(L, LUABIND_DECORATE_TYPE(key_type), 2); \
			typename container_type::iterator	it;							\
			\
			if ((index < 1) || (index > container->size()))					\
			{																\
				lua_pushnil(L);												\
				return 1;													\
			}																\
			it = container->begin();										\
			while(--index)													\
				++it;														\
			valConv.apply(L, (*it));										\
			return 1;														\
		}																	\
	};

LUABIND_STL_CONTAINER__INDEX(std::list)
LUABIND_STL_CONTAINER__INDEX(std::set)
LUABIND_STL_CONTAINER__INDEX(std::multiset)

#	undef LUABIND_STL_CONTAINER__INDEX

template <class Container_tt>
struct stl_container__next
{
	typedef typename stl_container_keyvalue<Container_tt>::key_type key_type;
	typedef typename stl_container_keyvalue<Container_tt>::value_type value_type;

	static int impl(lua_State* L)
	{
		/* 1: table
		 * 2: key
		 */
		default_converter< key_type >	keyConv;
		Container_tt* container = stl_adapter<Container_tt>::container_from_table(L, 1);
		key_type index;
		if((lua_gettop(L) < 2) || lua_isnil(L, 2))
			index = 0;
		else
			index = keyConv.apply(L, LUABIND_DECORATE_TYPE(key_type), 2);

		++index;
		if(index > container->size())
		{
			lua_pushnil(L);
			return 1;
		}

		keyConv.apply(L, index);
		lua_pushvalue(L, -1);
		lua_gettable(L, 1);
		return 2;
	}
};


template <class KeyType_tt, class ValType_tt >
struct stl_container__next< std::map<KeyType_tt, ValType_tt> >
{
	typedef KeyType_tt	key_type;
	typedef ValType_tt value_type;
	typedef std::map<key_type, value_type> container_type;

	static int impl(lua_State* L)
	{
		/* 1: table
		 * 2: key
		 */
		default_converter< value_type > valConv;
		default_converter< key_type >	keyConv;
		container_type* container = stl_adapter<container_type>::container_from_table(L, 1);
		typename container_type::iterator it;

		if((lua_gettop(L) < 2) || lua_isnil(L, 2))
		{
			it = container->begin();
		}
		else
		{
			key_type index;
			index = keyConv.apply(L, LUABIND_DECORATE_TYPE(key_type), 2);
			it = container->upper_bound(index);
		}

		if(it == container->end())
		{
			lua_pushnil(L);
			return 1;
		}

		keyConv.apply(L, (*it).first);
		valConv.apply(L, (*it).second);
		return 2;
	}
};

/**
 * metatable method, setter. table[key] = value;
 * Note that lua indexes are 1 based.
 */
// Allowing tables to be modified directly is going to cause grief.
// for instance what do do about a std::set?  changing an entry actually
// modifies the key so the table[4] = value has some very questionable
// meaning.  Especialy when there already exists an entry 4 in the set.
// 	template <class Container_tt>
// 	struct stl_container__newindex
// 	{
// 		typedef typename stl_container_keyvalue<Container_tt>::key_type key_type;
// 		typedef typename stl_container_keyvalue<Container_tt>::value_type value_type;
// 		typedef Container_tt container_type;
//
// 		static int impl(lua_State *L)
// 		{	/* 1: table object
// 			 * 2: key
// 			 * 3: value
// 			 */
// 			default_converter< value_type > valConv;
// 			default_converter< key_type >	keyConv;
// 			container_type *container = stl_adapter<container_type>::container_from_table(L, 1);
// 			key_type index = keyConv.apply(L, LUABIND_DECORATE_TYPE( key_type ), 2);
// 			value_type value = valConv.apply(L, LUABIND_DECORATE_TYPE( value_type ), 3);
//
// 			if (index < 1)
// 			{
// 				return luaL_error(L, "Index %d is out of range.", index);
// 			}
// 			if (index > container->size())
// 			{
// 				container->resize(index);
// 			}
// 			--index;
//
// 			container->at(index) = value;
// 			return 0;
// 		}
// 	};
//
// 	template <class KeyType_tt, class ValType_tt >
// 	struct stl_container__newindex< std::map<KeyType_tt, ValType_tt> >
// 	{
// 		typedef typename KeyType_tt	key_type;
// 		typedef typename ValType_tt value_type;
// 		typedef std::map<key_type, value_type> container_type;
//
// 		static int impl(lua_State *L)
// 		{
// 			default_converter< value_type > valConv;
// 			default_converter< key_type >	keyConv;
// 			container_type *container = stl_adapter<container_type>::container_from_table(L, 1);
// 			key_type index = keyConv.apply(L, LUABIND_DECORATE_TYPE( key_type ), 2);
// 			value_type value = valConv.apply(L, LUABIND_DECORATE_TYPE( value_type), 3);
// 			std::pair<container_type::iterator, bool> result;
//
// 			result = container.insert(container_type::value_type(index, value));
// 			if (!result.second)
// 				(*result.first)->second = value;
//
// 			return 0;
// 		}
// 	};
//
// #	define LUABIND_STL_CONTAINER__NEWINDEX(type)							\
// 	template <class ValType_tt>												\
// 	struct stl_container__newindex< type <ValType_tt> >						\
// 	{																		\
// 		typedef typename stl_container_keyvalue< type <ValType_tt> >::key_type key_type; \
// 		typedef typename stl_container_keyvalue< type <ValType_tt> >::value_type value_type; \
// 		typedef type <value_type> container_type;							\
// 																			\
// 		static int impl(lua_State *L)												\
// 		{																	\
// 			default_converter< value_type > valConv;						\
// 			default_converter< key_type >	keyConv;						\
// 			container_type *container = stl_adapter<container_type>::container_from_table(L, 1); \
// 			key_type index = keyConv.apply(L, LUABIND_DECORATE_TYPE( key_type ), 2); \
// 			value_type value = valConv.apply(L, LUABIND_DECORATE_TYPE( value_type), 3); \
// 			container_type::iterator it;									\
// 																			\
// 			if (index < 1)													\
// 			{																\
// 				return luaL_error(L, "Index %d is out of range.", index);	\
// 			}																\
// 			if (index > container->size())									\
// 			{																\
// 				container->resize(index);									\
// 			}																\
// 			it = container->begin();										\
// 			while (--index)													\
// 				++it;														\
// 			(*it) = value;													\
// 			return 0;														\
// 		}																	\
// 	};
//
// 	LUABIND_STL_CONTAINER__NEWINDEX(std::list)
// 	LUABIND_STL_CONTAINER__NEWINDEX(std::set)
// 	LUABIND_STL_CONTAINER__NEWINDEX(std::multiset)
//
// #	undef LUABIND_STL_CONTAINER__NEWINDEX
//

}
}

#endif
