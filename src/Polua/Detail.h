#ifndef __POLUA_DETAIL__
#define __POLUA_DETAIL__

namespace Polua
{
namespace detail
{
template<typename T, int I>
struct TypeWithIdx
{
	typedef T Type;
	static int const Index = I;
};

template<typename... Args>
struct TypeList
{
	TypeList() {}
};

template<int Index, typename T, typename... Args>
struct Add;

template<int Index, typename T, typename... Args>
struct Add<Index, T, TypeList<Args...> >
{
	static int const IdxInStack = -(Index + 1);
	typedef TypeList<Args..., TypeWithIdx<T, IdxInStack> > Type;
};

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

template<typename... Args>
struct ArgIdxListMaker;

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

template<typename TI>
static auto fromStack(lua_State* L)
-> decltype(Polua::fromstackAny<typename TI::Type>(L, TI::Index))
{
	return Polua::fromstackAny<typename TI::Type>(L, TI::Index);
}
}
}

#endif //__POLUA_DETAIL__