//! @file
//! @author Loïc HAMOT
#ifndef __POLUA_CORE__
#define __POLUA_CORE__

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <type_traits>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
	//#include "lualib.h"
}

//luaL_loadXXX => compile du code et l'ajoute sur la pile sous la forme d'une fonction
//getglobal global vers pile
//luaL_ref crée une référence(dans la table donnée) sur l'objet sur la pile, pop l'objet, et retourne la ref
//lua_rawgeti pousse sur la pile un objet reference dans la table donné
//une lib est une liste de pair nom -> cfunction (dans lua_newLib par example)
//luaL_setmetatable(L, tname); //Donne la metatable tname(pré-enregistré dans le registre) à l'objet sur la pile
//lua_getmetatable(L, index); //Met sur la pile la metatable de l'objet pointé


namespace Polua
{

//! Check qu'à la sortie, la pile lua a été changé du nombre d'item attendu
class CheckStack
{
	CheckStack(CheckStack const&);            //!< Class non-copiable
	CheckStack& operator=(CheckStack const&); //!< Class non-copiable

public:
	lua_State* const L;         //!< Données de l'interpreteur Lua
	int const expectedDiff;     //!< Difference attendue entre debut et fin
	int const startValue;       //!< Taille de la pile au debut
	std::string const funcName; //!< Nom de la fonction téstée

	//! ctor
	CheckStack(lua_State* L,          //!<Données de l'interpreteur Lua
	           int expDiff,           //!< Diff attendue entre debut et fin
	           char const* const func //!< Nom de la fonction téstée
	          ):
		L(L),
		expectedDiff(expDiff),
		startValue(lua_gettop(L)),
		funcName(func)
	{
	}

	//! Teste si la pile et été modifié comme attendu, et appel terminate sinon
	~CheckStack()
	{
		if(lua_gettop(L) != startValue + expectedDiff)
		{
			std::cout << funcName + " Bad stack managment" << std::endl;
			terminate();
		}
	}
};
#ifdef _DEBUG
#  define POLUA_CHECK_STACK(L, D) CheckStack checkStack(L, D, __FUNCTION__);
#else
//! @brief Check(en debug) qu'à la sortie,
//!   la pile lua a changé du nombre d'item attendu
#  define POLUA_CHECK_STACK(L, D) ;
#endif

//! Affiche le contenue de la pile lua dans la sortie standard
inline void printStack(lua_State* L)
{
	POLUA_CHECK_STACK(L, 0);
	int const nbArgs = lua_gettop(L);
	for(int i = 1; i <= nbArgs; ++i)
	{
		char const* const tostring = lua_tostring(L, i);
		std::cout << lua_typename(L, lua_type(L, i)) << " " <<
		          (tostring ? tostring : "") <<
		          std::endl;
	}
}

//! Exception signalant une erreur dans la code lua ou dans l'usage de Polua
struct Exception : std::runtime_error
{
	int errCode; //!< Code d'érreur lua, quand applicable
	//! ctor
	Exception(int err,                   //!< Code d'érreur lua, ou zero
	          std::string const& mess    //!< Message d'erreur
	         ):
		std::runtime_error(mess),
		errCode(err)
	{
	}
};


//! @brief Traits permettant de savoir
//!  si un type peut ètre considéré comme un type primitife pour lua
template<typename T>struct IsPrimitive
{
	static bool const value = false; //!< true si primitife (false par defaut)
	struct IsNot {};
};
#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<>struct IsPrimitive<bool    > {static bool const value = true; struct Is {};};

template<>struct IsPrimitive<int8_t  > {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<int16_t > {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<int32_t > {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<int64_t > {static bool const value = true; struct Is {}; };

template<>struct IsPrimitive<uint8_t > {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<uint16_t> {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<uint32_t> {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<uint64_t> {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<lua_CFunction> {static bool const value = true; struct Is {}; };
template<>struct IsPrimitive<std::string  > {static bool const value = true; struct Is {}; };

template<>struct IsPrimitive<double> {static bool const value = true; struct Is {};};
template<>struct IsPrimitive<float> {static bool const value = true; struct Is {};};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Lance une Polua::Exception si le code d'erreur n'est pas 0(OK)
inline void throwOnError(lua_State* L, int errCode)
{
	if(errCode)
	{
		char const* mess = lua_tostring(L, -1);
		std::string message = mess ? mess : "lua error without message";
		lua_pop(L, 1);
		throw Exception(errCode, message);
	}
}

//! @brief Stock un objet c++. Copie une rédérence.
//! Déstiné a etre crée en temps que userdata.
template<typename T>
class WrapperBase
{
	WrapperBase(WrapperBase const&);             //!< Non copyable
	WrapperBase& operator=(WrapperBase const&);  //!< Non copyable
public:
	WrapperBase() {}

	//! Destructeur virtuel
	virtual ~WrapperBase() = 0 {};

	//! Recupere le pointeur sur l'objet stoqué
	virtual T* getPtr() = 0;
};

//! Stock la copie d'un objet utilisateur.
template<typename T>
class CopyWrapper : public WrapperBase<T>
{
	T copy_;  //!< Copie de l'objet stocké

public:
	//! Construit l'objet stocké par copie ou autre
	template<typename ...Args>
	explicit CopyWrapper(Args const& ... args): copy_(args...) {}

	//! Recupere le pointeur sur l'objet stoqué
	virtual T* getPtr() {return &copy_;}
};

//! Stock une référence sur un objet utilisateur.
template<typename T>
class RefWrapper : public WrapperBase<T>
{
	T& ref_;  //!< stocke une référence sur l'objet

public:
	//! Prend la référence sur l'objet donné
	explicit RefWrapper(T& ref): ref_(ref) {}

	//! Recupere le pointeur sur l'objet stoqué
	virtual T* getPtr() {return &ref_;}
};


//! @brief Extrait un type C++ d'un userdata et retourne son pointeur.
//!
//! Appel luaL_error l'index donné ne référence pas un userdata
template<typename T>
static T* userdata_fromstack(lua_State* L, int idx)
{
	WrapperBase<T>* obj = static_cast<WrapperBase<T>*>(lua_touserdata(L, idx));
	if(!obj)
	{
		luaL_error(L, "Not a %s on the stack, as expected", typeid(T).name());
		return nullptr; // lightcheck returns nullptr if not found.
	}
	return obj->getPtr();		// pointer to T object
}

//! @brief Extrait un type C++ d'un userdata et retourne son pointeur.
//!
//! Appel luaL_error l'index donné ne référence pas un userdata
//! Appel luaL_error l'index donné ne référence un objet C++ du type attendu
template<typename T>
static T* check(lua_State* L, int narg)
{
	WrapperBase<T>* obj = static_cast<WrapperBase<T>*>(
	                        luaL_checkudata(L, narg, typeid(T).name()));
	if(!obj)
	{
		luaL_error(L, "Not a %s on the stack, as expected", typeid(T).name());
		return nullptr; // lightcheck returns nullptr if not found.
	}
	return obj->getPtr();		// pointer to T object
}

//! @brief Extrait un objet primitif de la pile
//! @remark Ne compile pas si T n'est pas primitif
template<typename T>
T fromstack(lua_State* L, int index)
{
	static_assert(false, "Can't compile with this type");
}
#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<>
inline bool fromstack<bool>(lua_State* L, int index)
{
	return lua_toboolean(L, index) != 0;
}

template<>
inline lua_CFunction fromstack<lua_CFunction>(lua_State* L, int index)
{
	return lua_tocfunction(L, index);
}

template<>
inline std::string fromstack<std::string>(lua_State* L, int index)
{
	char const* str = lua_tostring(L, index);
	return str ? str : "";
}

template<typename I>
I int_fromstack(lua_State* L, int index)
{
	int isnum = 0;
	lua_Integer const num = lua_tointegerx(L, index, &isnum);
	if(isnum == 0)
		return (I)luaL_error(
		         L,
		         "Expected an integer, but got a %s, at index %d on the stack",
		         lua_typename(L, lua_type(L, index)), index);
	if(num >= std::numeric_limits<I>::max() ||
	   num < std::numeric_limits<I>::min())
		return (I)luaL_error(
		         L, "Can't store value %d in type %s", num, typeid(I).name());
	return static_cast<I>(num);
}

template<typename U>
U uint_fromstack(lua_State* L, int index)
{
	int isnum = 0;
	lua_Unsigned const num = lua_tounsignedx(L, index, &isnum);
	if(isnum == 0)
		return (U)luaL_error(
		         L,
		         "Expected an unsigned but got a %s, at index %d on the stack",
		         lua_typename(L, lua_type(L, index)), index);
	if(num >= std::numeric_limits<U>::max() ||
	   num < std::numeric_limits<U>::min())
		return (U)luaL_error(
		         L, "Can't store value %d in type %s", num, typeid(U).name());
	return static_cast<U>(num);
}

#define POLUA_DECL_INT(I)                                                     \
	template<> inline I fromstack<I>(lua_State* L, int index)                 \
	{                                                                         \
		return int_fromstack<I>(L, index);                                    \
	}
POLUA_DECL_INT(int8_t)
POLUA_DECL_INT(int16_t)
POLUA_DECL_INT(int32_t)
POLUA_DECL_INT(int64_t)
#undef POLUA_DECL_INT

#define POLUA_DECL_UINT(U) \
	template<> inline U fromstack<U>(lua_State* L, int index)                 \
	{                                                                         \
		return uint_fromstack<U>(L, index);                                   \
	}
POLUA_DECL_UINT(uint8_t)
POLUA_DECL_UINT(uint16_t)
POLUA_DECL_UINT(uint32_t)
POLUA_DECL_UINT(uint64_t)
#undef POLUA_DECL_UINT

template<>
inline double fromstack<double>(lua_State* L, int index)
{
	int isnum = 0;
	double const num = lua_tonumberx(L, index, &isnum);
	if(isnum == 0)
		luaL_error(
		  L, "Not a number as expected, at index %d on the stack", index);
	return num;
}

template<>
inline float fromstack<float>(lua_State* L, int index)
{
	return static_cast<float>(fromstack<double>(L, index));
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


//! Pousse un type booleen dans la pile lua
inline void push(lua_State* L, bool value)
{
	lua_pushboolean(L, value);
}

//! Pousse un type lua_CFunction dans la pile lua
inline void push(lua_State* L, lua_CFunction value)
{
	lua_pushcfunction(L, value);
}

//! Pousse un type std::string dans la pile lua
inline void push(lua_State* L, std::string const& value)
{
	lua_pushstring(L, value.c_str());
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
inline void push(lua_State* L, double value) {lua_pushnumber(L, value);}
inline void push(lua_State* L, float  value) {lua_pushnumber(L, value);}

inline void push(lua_State* L, uint8_t  value) {lua_pushunsigned(L, value);}
inline void push(lua_State* L, uint16_t value) {lua_pushunsigned(L, value);}
inline void push(lua_State* L, uint32_t value) {lua_pushunsigned(L, value);}
inline void push(lua_State* L, uint64_t value) {lua_pushunsigned(L, static_cast<lua_Unsigned>(value));}
inline void push(lua_State* L,  int8_t  value) {lua_pushinteger(L, value);}
inline void push(lua_State* L,  int16_t value) {lua_pushinteger(L, value);}
inline void push(lua_State* L,  int32_t value) {lua_pushinteger(L, value);}
inline void push(lua_State* L,  int64_t value) {lua_pushinteger(L, static_cast<lua_Integer>(value));}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


//! Traits pour pousser dans la pile lua un objet
template<typename T, bool isPrim, bool isTemp>
struct Push;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// **************** Les type primitifs sont toujours copié ********************
//! Traits pour pousser dans la pile lua un ptr sur un primitif persistant
template<typename T>
struct Push<T*, true, false>
{
	static void push(lua_State* L, T* ptr)
	{
		Polua::push(L, *ptr);
	}
};
//! Traits pour pousser dans la pile lua un ptr sur un primitif temporaire
template<typename T> struct Push<T*, true, true> :
	public Push<T*, true, false> {};
//! Traits pour pousser dans la pile lua une réf sur un primitif persistant
template<typename T>
struct Push<T&, true, false>
{
	static void push(lua_State* L, T& ref)
	{
		Polua::push(L, ref);
	}
};
//! Traits pour pousser dans la pile lua une réf sur un primitif temporaire
template<typename T> struct Push<T&, true, true> :
	public Push<T&, true, false> {};
//! Traits pour pousser dans la pile lua un primitif persistant
template<typename T>
struct Push<T, true, false>
{
	static void push(lua_State* L, T const& obj)
	{
		Polua::push(L, obj);
	}
};
//! Traits pour pousser dans la pile lua un primitif temporaire
template<typename T> struct Push<T, true, true> :
	public Push<T, true, false> {};

// ********** Le userdata pointeur et ref sont stoké par pointeur *************

//! Pousse dans la pile lua une copie d'un type C++ en passant par CopyWrapper
template<typename T>
void pushUserdataCopy(lua_State* L, T const& val)
{
	POLUA_CHECK_STACK(L, 1);
	CopyWrapper<T>* usrdata =
	  (CopyWrapper<T>*)lua_newuserdata(L, sizeof(CopyWrapper<T>));
	luaL_setmetatable(L, typeid(T).name());
	new(usrdata) CopyWrapper<T>(const_cast<T&>(val));
}

//! Pousse dans la pile lua une ref sur un type C++ en passant par RefWrapper
template<typename T>
void pushUserdataRef(lua_State* L, T const& val)
{
	POLUA_CHECK_STACK(L, 1);
	RefWrapper<T>* usrdata =
	  (RefWrapper<T>*)lua_newuserdata(L, sizeof(RefWrapper<T>));
	luaL_setmetatable(L, typeid(T).name());
	new(usrdata) RefWrapper<T>(const_cast<T&>(val));
}

//! Traits pour pousser dans pile lua un ptr sur objet(non primitif) persistant
template<typename T>
struct Push<T*, false, false>
{
	static void push(lua_State* L, T* ptr)
	{
		if(ptr == nullptr)
			lua_pushnil(L);
		else
			pushUserdataRef(L, *ptr);
	}
};
//! Traits pour pousser dans pile lua un ptr sur objet(non primitif) temporaire
template<typename T> struct Push<T*, false, true> :
	public Push<T*, false, false> {};
//!Traits pour pousser dans pile lua une ref sur objet(non primitif) persistant
template<typename T>
struct Push<T&, false, false>
{
	static void push(lua_State* L, T& ref)
	{
		pushUserdataRef(L, ref);
	}
};


// ********** Le objet userdata persistent sont stoké par pointeur ************
//! Traits pour pousser dans pile lua un objet(non primitif) persistant
template<typename T>
struct Push<T, false, false>
{
	static void push(lua_State* L, T const& obj)
	{
		pushUserdataRef(L, obj);
	}
};

// ********** Le objet userdata temporaire sont copié dans la pile ************
//! Traits pour pousser dans pile lua un objet(non primitif) temporaire
template<typename T>
struct Push<T, false, true>
{
	static void push(lua_State* L, T const& obj)
	{
		pushUserdataCopy(L, obj);
	}
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Obtient un type T a partir de T*, T&, T const, ou T
template<typename T>
struct Normalize
{
	//! Type T mais sans reference ni pointeur ni const
	typedef typename std::remove_cv <
	typename std::remove_reference <
	typename std::remove_pointer<T>::type >::type >::type type;
};

//! Test si la version non ptr, non ref et non const de T est primitive
template<typename T>
struct BaseIsPrim : public IsPrimitive<typename Normalize<T>::type> {};
//! Pousse sur la pile un objet temporaire primitife
template<typename T>
void pushTemp(lua_State* L, T const& val, typename BaseIsPrim<T>::Is* = 0)
{
	Push<T, true, true>::push(L, val);
}
//! Pousse sur la pile un objet temporaire non primitife
template<typename T>
void pushTemp(lua_State* L, T const& val, typename BaseIsPrim<T>::IsNot* = 0)
{
	Push<T, false, true>::push(L, val);
}
//! Pousse sur la pile un objet persistant primitife
template<typename T>
void pushPers(lua_State* L, T const& val, typename BaseIsPrim<T>::Is* = 0)
{
	Push<T, true, false>::push(L, val);
}
//! Pousse sur la pile un objet persistant non primitife
template<typename T>
void pushPers(lua_State* L, T const& val, typename BaseIsPrim<T>::IsNot* = 0)
{
	Push<T, false, false>::push(L, val);
}


template<typename T, bool isPrim>
struct Fromstack;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// ***************** Le objet primitifs sont toujours copiés ******************
template<typename T>
struct Fromstack<T*, true>
{
	//! Les pointeurs sur des primitif ne sont pas géré
};

template<typename T>
struct Fromstack<T&, true>
{
	//! Les référence non constante sur des primitif ne sont pas géré
};

template<typename T>
struct Fromstack<T const&, true>
{
	//! Les référence constante sur des primitif sont retournées par copié
	typedef T result_type;
	static result_type fromstack(lua_State* L, int index)
	{
		return Polua::fromstack<typename Normalize<T>::type>(L, index);
	}
};

template<typename T>
struct Fromstack<T, true>
{
	typedef T result_type;
	static result_type fromstack(lua_State* L, int index)
	{
		return Polua::fromstack<T>(L, index);
	}
};

// ************** Les userdata sont récupéré par pointeur *********************
template<typename T>
struct Fromstack<T*, false>
{
	typedef T* result_type;
	static result_type fromstack(lua_State* L, int index)
	{
		return check<T>(L, index);
	}
};

template<typename T>
struct Fromstack<T&, false>
{
	typedef T& result_type;
	static result_type fromstack(lua_State* L, int index)
	{
		return *check<T>(L, index);
	}
};

template<typename T>
struct Fromstack<T, false>
{
	typedef T& result_type;
	static result_type fromstack(lua_State* L, int index)
	{
		return *check<T>(L, index);
	}
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Récupère un objet depuit la pile lua a l'index donné
template<typename T>
typename Fromstack<T, true>::result_type
fromstackAny(lua_State* L, int index, typename BaseIsPrim<T>::Is* = 0)
{
	return Fromstack<T, true>::fromstack(L, index);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <
typename T,
         class = typename std::enable_if<std::is_enum<T>::value>::type >
T fromstackEnum(lua_State* L, int index)
{
	return static_cast<T>(Fromstack<std::intmax_t, true>::fromstack(L, index));
}
template <
typename T,
         class = typename std::enable_if < !std::is_enum<T>::value >::type >
typename Fromstack<T, false>::result_type
fromstackEnum(lua_State* L, int index)
{
	return Fromstack<T, false>::fromstack(L, index);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Récupère un enum depuit la pile lua a l'index donné
template<typename T>
typename Fromstack<T, std::is_enum<T>::value>::result_type
fromstackAny(
  lua_State* L,                      //!< Données de l'interpréteur lua
  int index,                         //!< Index de l'obj désiré dans la pile
  typename BaseIsPrim<T>::IsNot* = 0 //!< Juste pour faire marcher le SFINAE
)
{
	return fromstackEnum<T>(L, index);
}

//! @brief Ajoute une valeur dans la table, à la clé donné
//! @pre La metatable est au sommet de la pile
template<typename T>
void setInMetatable(
  lua_State* L,            //!< Données de l'interpréteur lua
  std::string const& name, //!< Nom de la donnée dans la metatable
  T const& val             //!< Donée a ajouter dans la metatable
)
{
	POLUA_CHECK_STACK(L, 0);
	Polua::pushTemp(L, val);           //push la valeur
	lua_setfield(L, -2, name.c_str()); //metatable[name] = val; puis pop la clé
}

//! @brief Ajoute un lua_CFunction dans la table, à la clé donné
//! @pre La metatable est au sommet de la pile
inline void setInMetatable(
  lua_State* L,            //!< Données de l'interpréteur lua
  std::string const& name, //!< Nom de la function dans la metatable
  lua_CFunction val        //!< Pointeur sur function C appelable par lua
)
{
	POLUA_CHECK_STACK(L, 0);
	Polua::push(L, val);               //push la lua_CFunction
	lua_setfield(L, -2, name.c_str()); //metatable[name] = val; puis pop la clé
}

}

#endif //__POLUA_CORE__