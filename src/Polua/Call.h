//! @file
//! @author Lo�c HAMOT
#ifndef __POLUA_CALL__
#define __POLUA_CALL__

#include <string>

#include "Core.h"

extern "C"
{
#include "lua.h"
}

namespace Polua
{
//! Class servant � appeler une function lua depuit le C++
class Caller
{
	lua_State* L;            //!< Donn�e de l'interpr�teur lua

	//! Pousse dans la pile une liste d'argument
	template<typename First, typename... Others>
	void pushArgs(First const& first, Others const& ... others)
	{
		pushPers(L, first);
		pushArgs(others...);
	}
	void pushArgs() {}

public:
	Caller(lua_State* L): L(L) {}

	//! Appel une function sur la pile lua, qui ne retourne pas de r�sultat
	//! @throw Polua::Exception en cas d'erreur
	template<typename ...Args>
	void call(Args const& ... args)
	{
		POLUA_CHECK_STACK(L, -1);
		pushArgs(args...);
		throwOnError(L, lua_pcall(L, sizeof...(Args), 0, 0));
	}

	//! Appel une function sur la pile lua, qui retourne un r�sultat
	//! @throw Polua::Exception en cas d'erreur
	//! @return r�sultat de la function lua, et d�termin� par R
	template<typename R, typename ...Args>
	R call(Args const& ... args)
	{
		POLUA_CHECK_STACK(L, -1);
		pushArgs(args...);
		throwOnError(L, lua_pcall(L, sizeof...(Args), 1, 0));
		R res = fromstackAny<R>(L, -1);
		lua_pop(L, 1);
		return res;
	}
};
}

#endif //__POLUA_CALL__