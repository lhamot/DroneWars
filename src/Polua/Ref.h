#ifndef __POLUA_REF__
#define __POLUA_REF__

#include "Core.h"
#include "Call.h"

namespace Polua
{

//! Référence un objet stoké dans les registre de lua
class Ref
{
	Ref(Ref const&);
	Ref& operator=(Ref const&);

	lua_State* L; //!< Données de l'interpreteur lua
	int ref;      //!< Identifiant de l'obj dans les registres (0 si invalide)

public:
	inline explicit Ref(lua_State* L):
		L(L),
		ref(luaL_ref(L, LUA_REGISTRYINDEX))
	{
	}

	inline Ref(lua_State* L, std::string const& name): L(L), ref(LUA_REFNIL)
	{
		POLUA_CHECK_STACK(L, 0);
		lua_getglobal(L, name.c_str());       //Pousse la variable global name
		ref = luaL_ref(L, LUA_REGISTRYINDEX); //Crée la ref et pop l'objet
	}

	//! Supprime la référence si il y en as une
	inline ~Ref()
	{
		POLUA_CHECK_STACK(L, 0);
		if(ref >= 0)
			luaL_unref(L, LUA_REGISTRYINDEX, ref);
	}

	//! Pousse l'objet référencé dans la pile
	inline void push() const
	{
		POLUA_CHECK_STACK(L, 1);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}

	//! Retourne le type de l'objet référence (voir lua_type)
	inline int type() const
	{
		POLUA_CHECK_STACK(L, 0);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref); //Pousse l'objet sur la pile
		int const type = lua_type(L, -1);       //Recupère son type
		lua_pop(L, 1);                          //pop l'objet de la pile
		return type;
	}

	//! Test si la référence est valide
	inline bool is_valid() const
	{
		return ref >= 0;
	}

	lua_State* state() {return L;}

	//! Appel this en tant que function qui ne retourne pas de résultat
	//! @throw Polua::Exception en cas d'erreur
	template<typename ...Args>
	void call(Args const& ... args)
	{
		POLUA_CHECK_STACK(L, 0);
		if(is_valid() == false)
			BOOST_THROW_EXCEPTION(Polua::Exception(0, "Try to call invalid reference"));
		push();
		Caller caller(L);
		return caller.call(args...);
	}

	//! Appel this en tant que function qui retourne un résultat
	//! @throw Polua::Exception en cas d'erreur
	//! @return résultat de la function lua, et déterminé par R
	template<typename R, typename ...Args>
	R call(Args const& ... args)
	{
		POLUA_CHECK_STACK(L, 0);
		if(is_valid() == false)
			BOOST_THROW_EXCEPTION(Polua::Exception(0, "Try to call invalid reference"));
		push();
		Caller caller(L);
		return caller.call<R>(args...);
	}

	template<typename T>
	T get()
	{
		POLUA_CHECK_STACK(L, 0);
		push();
		T obj = Polua::fromstackAny<T>(L, -1);
		lua_pop(L, 1);
		return obj;
	}
};

//! Référence un obj stoké dans les registre de lua (avec sémentique de valeur)
typedef std::shared_ptr<Ref> object;

//! Crée une référence sur l'objet en haut de la pile, et le pop
inline object ref(lua_State* L)
{
	POLUA_CHECK_STACK(L, 0);
	return std::make_shared<Ref>(L);
}

//! Crée une référence un objet présent dans les variables globals
inline object refFromName(lua_State* L, std::string const& name)
{
	POLUA_CHECK_STACK(L, 0);
	return std::make_shared<Ref>(L, name);
}

}

#endif //__POLUA_REF__