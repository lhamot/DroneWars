//! @file
//! @author Loïc HAMOT
#ifndef __POLUA_DETAIL__
#define __POLUA_DETAIL__

namespace Polua
{
namespace detail
{
//! Contient un type et un index static
template<typename T, int I>
struct TypeWithIdx
{
	typedef T Type;             //!< Le type
	static int const Index = I; //!< L'index
};

//! Liste de type static
template<typename... Args>
struct TypeList
{
	TypeList() {}
};

//! Ajout a la liste de type un nouveau TypeWithIdx
template<int Index, typename T, typename... Args>
struct Add;

//! Implementation de Add
template<int Index, typename T, typename... Args>
struct Add<Index, T, TypeList<Args...> >
{
	//! Index dans la pile lua
	static int const IdxInStack = -(Index + 1);
	//! On ajoute a la liste de TypeWithIdx un nouveau TypeWithIdx
	typedef TypeList<Args..., TypeWithIdx<T, IdxInStack> > Type;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template<typename... Args>
struct ArgIdxListMaker2;


template<typename... Outputs>
struct ArgIdxListMaker2<TypeList<Outputs...> >
{
	typedef TypeList<Outputs...> Type;
};

template<typename ArgList, typename First, typename... Inputs>
struct ArgIdxListMaker2<ArgList, First, Inputs...> :
	public ArgIdxListMaker2 <
typename Add<sizeof...(Inputs), First, ArgList>::Type, Inputs... >
{
};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Génere une liste de TypeWithIdx avec pour chaque type donné en entré,
//! l'index (dans la pile lua) plus le type.
template<typename... Args>
struct ArgIdxListMaker;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template<typename First, typename... Inputs>
struct ArgIdxListMaker<First, Inputs...> :
	public ArgIdxListMaker2 <
typename Add<sizeof...(Inputs), First, TypeList<> >::Type, Inputs... >
{
};

template<>
struct ArgIdxListMaker<>
{
	typedef TypeList<> Type;
};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//! Extrait un objet de la pile lua, a partir du TypeWithIdx donné
template<typename TI>
static auto fromStack(lua_State* L)
-> decltype(Polua::fromstackAny<typename TI::Type>(L, TI::Index))
{
	return Polua::fromstackAny<typename TI::Type>(L, TI::Index);
}
}
}

#endif //__POLUA_DETAIL__