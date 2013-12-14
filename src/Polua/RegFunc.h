//! @file
//! @author Loïc HAMOT
#ifndef __POLUA_REGFUNC__
#define __POLUA_REGFUNC__

#include <string>
#include "Detail.h"

extern "C"
{
#include "lua.h"
}

namespace Polua
{
//! Enregistre un lua_CFunction dans la memoire global de lua
void regFunc(lua_State* L,             //!< Donnée de l'interpréteur lua
             std::string const& name,  //!< Nom de la function dans lua
             lua_CFunction func        //!< function lua
            )
{
	POLUA_CHECK_STACK(L, 0);
	lua_pushcfunction(L, func);     //Pousse la function sur la pile
	lua_setglobal(L, name.c_str()); //La déplace dans les globals, nommé name
}

//! @brief Enregistre un wraper de function C++ dans les global lua
//!
//! Enregistre un lua_CFunction closure avec le pointeur d'une fonction c++
//! dans la memoire global de lua. Le lua_CFunction est généralement un wrapper
//! pour appeler la function c++.
template<typename F>
void regFunc(lua_State* L,             //!< Donnée de l'interpréteur lua
             std::string const& name,  //!< Nom de la function dans lua
             lua_CFunction caller,     //!< wrapper appelant la function c++
             F funcPtr                 //!< Pointeur de fonction c++
            )
{
	POLUA_CHECK_STACK(L, 0);
	lua_pushlightuserdata(L, funcPtr); // Pousse la function c++ sur la pile
	lua_pushcclosure(L, caller, 1);    // pop la function et pousse le closure
	lua_setglobal(L, name.c_str());    // Le transfert dans la memoire global
}

//! @brief Contient des fonctions utilisée par regFunc
//!   pour enregistrer des function c++ dans lua
namespace Function
{
//! @brief Wrapper qui crée une lua_CFunction appelant une fonction C++ donnée
template<typename T>
struct Caller;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

//! @brief Wrapper servant à créer une lua_CFunction appelant
//! une fonction C++ donnée et ayant une valeur de retour
template<typename R, typename... Args>
struct Caller<R(*)(Args...)>
{
  typedef R(*FuncPtr)(Args...);

  template<typename... ArgIdxList>
  static int call2(
    lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	FuncPtr ptr = (FuncPtr)lua_touserdata(L, lua_upvalueindex(1));
	pushTemp(L, ptr(fromStack<ArgIdxList>(L)...));
	return 1;
}

static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};

//! @brief Wrapper servant à créer une lua_CFunction appelant
//! une fonction C++ donnée sans valeur de retour
template<typename... Args>
struct Caller<void(*)(Args...)>
{
  typedef void(*FuncPtr)(Args...); //!< Pointeur sur la fonction C++

  template<typename... ArgIdxList>
  static int call2(lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	FuncPtr ptr = (FuncPtr)lua_touserdata(L, lua_upvalueindex(1));
	ptr(fromStack<ArgIdxList>(L)...);
	return 0;
}

static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

}


//! @brief Enregistre une function c++ dans les variables global de lua
//!   en la rendant appelable avec le nom donnée
template<typename F>
void regFunc(lua_State* L, std::string const& name, F funcPtr)
{
	POLUA_CHECK_STACK(L, 0);
	regFunc(L, name, &Function::Caller<F>::call, funcPtr);
}
}

#endif //__POLUA_REGFUNC__