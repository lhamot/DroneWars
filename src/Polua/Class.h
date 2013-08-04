//! @file
//! @author Lo�c HAMOT
#ifndef __POLUA_CLASS__
#define __POLUA_CLASS__

#include <cassert>
#include <vector>
#include <tuple>

#include "Core.h"
#include "Detail.h"

extern "C"
{
#include "lua.h"
}

namespace Polua
{
//! @brief Contient un pointeur sur une methode avec son wrapper lua
//! (util pour le binding de methodes)
struct Methode
{
	lua_CFunction caller;   //!< permet a lua d'appeler la methode
	void* func;             //!< Pointeur sur la m�thode a appeler
	Methode(): caller(nullptr), func(nullptr) {}
	Methode(lua_CFunction caller, void* func): caller(caller), func(func) {}
};
//! @brief Pointeur de fonction servant a excecuter un Setter ou Getter
//! (util pour le binding des propri�t�s)
typedef int (*Xetter)(lua_State*, void*);
struct Member
{
	Methode methode;     //!< Wrapper de methode (peut �tre null)
	Xetter getter;       //!< Wrapper de getter (peut �tre null)
	Xetter setter;       //!< Wrapper de setter (peut �tre null)
	void* attibPtr;      //!< Pointeur sur l'attribut des Xetter
	explicit Member(Methode const f):
		methode(f), getter(nullptr), setter(nullptr), attibPtr(nullptr) {}
	Member(void* attibPtr, Xetter get, Xetter set = nullptr):
		getter(get), setter(set), attibPtr(attibPtr) {assert(get);}
};

//! Surcharge de setInMetatable pour le Polua::Member
inline void setInMetatable(
  lua_State* L, std::string const& name, Member const& member)
{
	POLUA_CHECK_STACK(L, 0);
	Member* ptr = (Member*)lua_newuserdata(L, sizeof(Member));
	new(ptr) Member(member);
	lua_setfield(L, -2, name.c_str()); //metatable[name] = val; puis pop la cl�
}


namespace ClassHelpers
{

//! Convertie un void* dans le type v�ritable d'une methode menbre
template<typename O>
static O toMember(void* in)
{
	static_assert(sizeof(void*) >= sizeof(O), "Can't cast");
	union
	{
		void* input;
		O output;
	};
	input = in;
	return output;
}

//! Convertie le type v�ritable d'une methode membre en void*
template<typename I>
void* memberToVoid(I in)
{
	static_assert(sizeof(void*) >= sizeof(I), "Can't cast");
	union
	{
		I input;
		void* output;
	};
	input = in;
	return output;
}


//! @brief Extrait un objet C++ et une methode de la pile
//! (util quand lua appel une methode c++)
template<typename Obj, typename MPtr>
struct ObjAndMethode
{
	Obj* object;
	MPtr methode;

	ObjAndMethode(lua_State* L):
		object(userdata_fromstack<Obj>(L, lua_upvalueindex(1))),
		methode(toMember<MPtr>(lua_touserdata(L, lua_upvalueindex(2))))
	{
	}
};


//! Obtient le type voulue(ref ou ptr) a partir un type forcement pointeur
template<class T> struct PtrToType     {static T& get(T* val) {return *val;}};
template<class T> struct PtrToType<T&> {static T& get(T* val) {return *val;}};
template<class T> struct PtrToType<T*> {static T* get(T* val) {return val;}};

//! Cr�e la lua_CFunction qui appelera une methode membre C++ ayant un retour
template<typename R, typename... Args>
struct MemCallerR;

template<typename R, typename T, typename... Args>
struct MemCallerR<R(T::*)(Args...)>
{
  typedef R(T::*FuncPtr)(Args...);

  template<typename... ArgIdxList>
  static int call2(lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	ObjAndMethode<typename Normalize<T>::type, FuncPtr> OMP(L);
	Polua::pushTemp(
	  L,
	  (OMP.object->*OMP.methode)(fromStack(L, ArgIdxList())...));
	return 1;
}

static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};


template<typename R, typename T, typename ...Args>
struct MemCallerR<R(*)(T, Args...)>
{
  typedef R(*FuncPtr)(T, Args...);

  template<typename... ArgIdxList>
  static int call2(lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	ObjAndMethode<typename Normalize<T>::type, FuncPtr> OMP(L);
	Polua::pushTemp(
	  L,
	  OMP.methode(PtrToType<T>::get(OMP.object),
	              fromStack(L, ArgIdxList())...));
	return 1;
}
static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};

//! Cr�e la lua_CFunction qui appelera une methode membre C++ sans retour
template<typename... Args>
struct MemCaller;

template<typename T, typename... Args>
struct MemCaller<void(T::*)(Args...)>
{
  typedef void(T::*FuncPtr)(Args...);

  template<typename... ArgIdxList>
  static int call2(lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	ObjAndMethode<typename Normalize<T>::type, FuncPtr> OMP(L);
	(OMP.object->*OMP.methode)(fromStack(L, ArgIdxList())...);
	return 0;
}
static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};


template<typename T, typename... Args>
struct MemCaller<void(*)(T, Args...)>
{
  typedef void(*FuncPtr)(T, Args...);

  template<typename... ArgIdxList>
  static int call2(lua_State* L, detail::TypeList<ArgIdxList...> const&)
{
	using namespace detail;
	ObjAndMethode<typename Normalize<T>::type, FuncPtr> OMP(L);
	OMP.methode(PtrToType<T>::get(OMP.object),
	            fromStack(L, ArgIdxList())...);
	return 1;
}
static int call(lua_State* L)
{
	return call2(L, detail::ArgIdxListMaker<Args...>::Type());
}
};

}

//! Traits pour indexer(utiliser []) le type T
template<typename T>
struct Indexer
{
	static int get(lua_State* L)
	{
		return luaL_error(L, "Can't index type %s", typeid(T).name());
	}

	static int set(lua_State* L)
	{
		return luaL_error(L, "Can't index type %s", typeid(T).name());
	}
};


//! Traits pour it�rer le type T
template<typename T>
struct IPairs
{
	static int ipairs(lua_State*)
	{
		return 0;
	}
	static int pairs(lua_State*)
	{
		return 0;
	}
};


//! Permet d'enregistrer une class c++ dans lua
template<typename T>
class Class
{
	Class(Class const&);
	Class& operator=(Class const&);

	lua_State* L;             //!< Donn�e de l'interpreteur lua
	std::string const& name;  //!< Nom de la class dans lua

	//! metafunction lua "__call" pour le(s) constructeur
	static int __call(lua_State* L)
	{
		//Dans la pile: 1 = objet
		size_t const nbArgs = lua_gettop(L) - 1;
		char funcName[50];
		std::sprintf(funcName, "ctor%u", nbArgs);
		lua_getmetatable(L, 1);      //Pousse la metatable du type
		lua_pushstring(L, funcName); //Pousse le nom du constructeur
		lua_rawget(L, -2);		     //Pousse le ctor (� la place du nom)
		if(lua_type(L, -1) == LUA_TFUNCTION)
		{
			lua_CFunction ctor = lua_tocfunction(L, -1); //On recup le ctor
			lua_pop(L, 2);                               //On le vire, ac la mt
			return ctor(L);
		}
		else
		{
			lua_pop(L, 2); // On pop le ctor(qui n'en est pas un ) et ma mt
			return luaL_error(
			         L, "Can't found constructor with %d argument(s)", nbArgs);
		}
	}

	//! metafunction lua "__index" pour les valeur enum (statique donc)
	static int __enumindex(lua_State* L)
	{
		POLUA_CHECK_STACK(L, 1);
		//Dans la pile: 1 = objet, 2 = nom de la valeur
		lua_getmetatable(L, 1);  // Pousse la metatable du type
		lua_pushvalue(L, 2);	 // Pousse le nom
		lua_rawget(L, -2);		 // Pousse la valeur, a la place du nom

		intmax_t const val = lua_tointeger(L, -1); // Recup la valeur
		lua_pop(L, 2);           // Pop le metatable et la valeur
		lua_pushinteger(L, val); // Pousse la valeur sur la pile
		return 1;
	}

	//! metafunction lua "__index" pour les methodes et getters
	static int __index(lua_State* L)
	{
		//Dans la pile: 1 = objet, 2 = nom du membre
		lua_getmetatable(L, 1); // Look up the index of a name
		lua_pushvalue(L, 2);	// Push the name
		lua_rawget(L, -2);		// Get the function

		int const type = lua_type(L, -1);
		if(type == LUA_TUSERDATA)
		{
			Member* member = (Member*)lua_touserdata(L, -1);
			lua_pop(L, 2);                                      // Pop metatable and Xetter/methode
			if(member->methode.caller)                          // Si c'est une methode
			{
				lua_pushvalue(L, 1);                            //   Remet l'objet sur le dessu de la pile
				lua_pushlightuserdata(L, member->methode.func); //   Et la fonction par dessu
				lua_pushcclosure(L, member->methode.caller, 2); //   Retourne l'appelant avec deux arguments pr�definit
				return 1;
			}
			else if(member->getter)
				return member->getter(
				         L, ClassHelpers::memberToVoid(member->attibPtr));
			else
			{
				std::string name = Polua::fromstack<std::string>(L, 2);
				return luaL_error(L, "Can't get property %s", name.c_str());
			}
		}
		else if(type == LUA_TNUMBER)
		{
			lua_Integer const value = lua_tointeger(L, -1); //On recup la valeur
			lua_pop(L, 2);                                  //Pop metatable and Xetter/methode
			lua_pushinteger(L, value);                      //On met la valeur sur la pile
			return 1;                                       //Et on dit qu'on as une valeur retourn�e
		}
		else
		{
			lua_pop(L, 2);    // Pop metatable and Xetter/methode
			return Indexer<T>::get(L);
		}
	}

	//! metafunction lua "__newindex" pour les setters
	static int __newindex(lua_State* L)
	{
		//1 object
		//2 key
		lua_getmetatable(L, 1); // Look up the index of a name
		lua_pushvalue(L, 2);	// Push the name
		lua_rawget(L, -2);		// Get the member

		if(lua_isuserdata(L, -1))
		{
			Member* member = (Member*)lua_touserdata(L, -1);
			lua_pop(L, 2);    // Pop metatable and member
			if(member->setter)
				return member->setter(
				         L, ClassHelpers::memberToVoid(member->attibPtr));
			else
			{
				std::string name = Polua::fromstack<std::string>(L, 2);
				return luaL_error(L, "Can't set property %s", name.c_str());
			}
		}
		else
		{
			lua_pop(L, 2);    // Pop metatable and member
			return Indexer<T>::set(L);
		}
	}


	//! Construit le lua_cfunction qui appelera le constructeur
	template<typename... Args>
	struct Ctor
	{
		template<typename T, int I>
		static auto ctorFromStack(lua_State* L,
		                          detail::TypeWithIdx<T, I> const&)
		-> decltype(Polua::fromstackAny<T>(L, I))
		{
			return Polua::fromstackAny<T>(L, I - 1);
		}

		template<typename... ArgIdxList>
		static int constructor2(
		  lua_State* L,
		  detail::TypeList<ArgIdxList...> const&)
		{
			CopyWrapper<T>* ptr =
			  (CopyWrapper<T>*)lua_newuserdata(L, sizeof(CopyWrapper<T>));
			luaL_setmetatable(L, typeid(T).name());
			new(ptr) CopyWrapper<T>(
			  ctorFromStack(L, detail::ArgIdxList())...);
			return 1;
		}

		//! lua_cfunction qui appelera le constructeur
		static int constructor(lua_State* L)
		{
			return constructor2(
			         L, detail::ArgIdxListMaker<Args...>::Type());
		};
	};

	//! metamethode "__gc" appel� par lua pour detruire le userdata
	static int destructor(lua_State* L)
	{
		WrapperBase<T>* obj = static_cast<WrapperBase<T>*>(
		                        luaL_checkudata(L, -1, typeid(T).name()));
		if(!obj)
			return luaL_error(
			         L, "Not a %s on the stack as expected", typeid(T).name());
		else
		{
			obj->~WrapperBase<T>();
			return 0;
		}
	}

	//! metamethode "__tostring" appel� par lua pour afficher le userdata
	static int __tostring(lua_State* L)
	{
		T* obj = userdata_fromstack<T>(L, -1);
		if(obj)
			lua_pushfstring(L, "%s (%p)", typeid(T).name(), (void*)obj);
		else
			lua_pushstring(L, "Empty object");
		return 1;
	}

	//! metamethode "__eq" appel� par lua tester l'egalit� de deux T
	static int __eq(lua_State* L)
	{
		POLUA_CHECK_STACK(L, 1);
		T const& a1 = Polua::fromstackAny<T>(L, 1);
		T const& a2 = Polua::fromstackAny<T>(L, 2);
		lua_pushboolean(L, a1 == a2);
		return 1;
	}

	//! Met sur la pile la metatable statique de T (et la construit au besoin)
	void pushMetatable()
	{
		POLUA_CHECK_STACK(L, 1)
		lua_getglobal(L, name.c_str());                                        // Pousse la metatable statique de T
		if(lua_isnil(L, -1))                                                   // Si nil � �t� pouss�, on va construir la mt
		{
			lua_pop(L, 1);                                                     //   Pop le nil
			lua_newuserdata(L, 1);                                             //   Pousse un nouveau userdata
			luaL_newmetatable(L, name.c_str());                                //   Pouse une nouvelle metatable au nom de T
			setInMetatable(L, "__index", &Class<T>::__enumindex);              //   Met la metamethode __index(pour le valeur statics) dans la mt
			setInMetatable(L, "__call", &Class<T>::__call);                    //   Met la metamethode __call(pour les ctor)  dans la mt
			lua_setmetatable(L, -2);                                           //   Associe la mt au userdata(et pop la)
			lua_setglobal(L, name.c_str());                                    //   D�place le userdata dans les globals
			lua_getglobal(L, name.c_str());                                    //   Pousse le userdata sur la pile
		}
	}

	//! Permet de stoker un getter universel dans un Polua::Member, dans lua
	template<typename A>
	static int getter(lua_State* L, void* voidMemberPtr)
	{
		A T::*memberPtr = ClassHelpers::toMember<A T::*>(voidMemberPtr);
		T* ptr = userdata_fromstack<T>(L, 1);
		pushPers(L, ptr->*memberPtr);
		return 1;
	}

	//! Permet de stoker un setter universel dans un Polua::Member, dans lua
	template<typename A>
	static int setter(lua_State* L, void* voidMemberPtr)
	{
		A T::*memberPtr = ClassHelpers::toMember<A T::*>(voidMemberPtr);
		T* ptr = userdata_fromstack<T>(L, 1);
		ptr->*memberPtr = Polua::fromstackAny<A>(L, -1);
		return 0;
	}

	//! Ajoute un Polua::Methode au type T
	Class& methode(std::string const& name, Methode const& methode)
	{
		setInMetatable(L, name, Member(methode, LuaCFunc(), LuaCFunc()));
		return *this;
	}

	template<typename R, typename... Args>
	struct GetFuncTypeR
	{
		typedef R(T::*MemberFunc)(Args...);
		typedef R(*ExternFunc)(Args...);
	};
	template<typename... Args>
	struct GetFuncType
	{
		typedef void(T::*MemberFunc)(Args...);
		typedef void(*ExternFunc)(Args...);
	};


public:
	Class(lua_State* state,            //!< Donn�es de l'interpr�teur lua
	      std::string const& type_name //!< Nom du type dans lua
	     ):
		L(state),
		name(type_name)
	{
		//Cr�e une metatable, avec les fonctions de mon type,
		//dans le registre et la pousse dans la pile
		luaL_newmetatable(L, typeid(T).name());
		setInMetatable(L, "__gc", &Class<T>::destructor);
		setInMetatable(L, "__tostring", &Class<T>::__tostring);
		setInMetatable(L, "__newindex", &__newindex);
		setInMetatable(L, "__index", &__index);
		setInMetatable(L, "__ipairs", &IPairs<T>::ipairs);
		setInMetatable(L, "__pairs", &IPairs<T>::pairs);
	}

	//! Ajoute un constructeur au type T
	template<typename... Args>
	Class& ctor()
	{
		POLUA_CHECK_STACK(L, 0);
		pushMetatable();
		lua_getmetatable(L, -1);
		size_t const nbArgs = sizeof...(Args);
		char funcName[50];
		std::sprintf(funcName, "ctor%u", nbArgs);
		setInMetatable(L, funcName, &Ctor<Args...>::constructor);
		lua_pop(L, 2);
		return *this;
	}


	//! Ajoute une valeur statique au type T
	Class& enumValue(std::string const& name, lua_Integer value)
	{
		POLUA_CHECK_STACK(L, 0);
		pushMetatable();
		lua_getmetatable(L, -1);
		setInMetatable(L, name.c_str(), value);
		lua_pop(L, 2);
		return *this;
	}

	//! Ajoute un op�rateur d'�galit� au type T
	Class& opEqual()
	{
		POLUA_CHECK_STACK(L, 0);
		setInMetatable(L, "__eq", &Class<T>::__eq);
		return *this;
	}

	//! Ajoute une propri�t� au type T
	template<typename A>
	Class& property(std::string const& name, A T::*memberPtr)
	{
		Xetter get = &getter<A>;
		Xetter set = &setter<A>;
		setInMetatable(
		  L, name, Member(ClassHelpers::memberToVoid(memberPtr), get, set));
		return *this;
	}

	//! Ajoute une propri�t� en lecture seul au type T
	template<typename A>
	Class& read_only(std::string const& name, A T::*memberPtr)
	{
		Xetter get = &getter<A>;
		Xetter set = nullptr;
		setInMetatable(
		  L, name, Member(ClassHelpers::memberToVoid(memberPtr), get, set));
		return *this;
	}

	//! Ajoute une methode au type T, a partir d'un lua_CFunction
	Class& methode(std::string const& name, lua_CFunction methode)
	{
		setInMetatable(L, name, Member(methode, LuaCFunc(), LuaCFunc()));
		return *this;
	}

	//! Ajoute une methode au type T, a partir d'une vrai methode
	template<typename... Args>
	Class& methode(std::string const& name, void(T::*methode)(Args...))
	{
		typedef typename GetFuncType<Args...>::MemberFunc FuncPtr;
		Methode closure(&ClassHelpers::MemCaller<FuncPtr>::call,
		                ClassHelpers::memberToVoid(methode));
		setInMetatable(L, name, Member(closure));
		return *this;
	}

	//! Ajoute une methode au type T, a partir d'une fonction libre
	template<typename... Args>
	Class& methode(std::string const& name, void(*methode)(Args...))
	{
		typedef typename GetFuncType<Args...>::ExternFunc FuncPtr;
		Methode closure(&ClassHelpers::MemCaller<FuncPtr>::call,
		                ClassHelpers::memberToVoid(methode));
		setInMetatable(L, name, Member(closure));
		return *this;
	}

	//! Ajoute une methode au type T, a partir d'une vrai methode
	template<typename R, typename... Args>
	Class& methode(std::string const& name, R(T::*methode)(Args...))
	{
		typedef typename GetFuncTypeR<R, Args...>::MemberFunc FuncPtr;
		Methode closure(&ClassHelpers::MemCallerR<FuncPtr>::call,
		                ClassHelpers::memberToVoid(methode));
		setInMetatable(L, name, Member(closure));
		return *this;
	}

	//! Ajoute une methode au type T, a partir d'une fonction libre
	template<typename R, typename... Args>
	Class& methode(std::string const& name, R(*methode)(Args...))
	{
		typedef typename GetFuncTypeR<R, Args...>::ExternFunc FuncPtr;
		Methode closure(&ClassHelpers::MemCallerR<FuncPtr>::call,
		                ClassHelpers::memberToVoid(methode));
		setInMetatable(L, name, Member(closure));
		return *this;
	}
};
}

#endif //__POLUA_CLASS__