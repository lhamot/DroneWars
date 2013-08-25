#include "Polua/Call.h"
#include "Polua/Class.h"
#include "Polua/Core.h"
#include "Polua/RegFunc.h"
#include "Polua/State.h"


#include <iostream>
#include <vector>

struct StructForEnum
{
	enum Enum
	{
		BLIP
	};
};

typedef StructForEnum::Enum StructForEnum_Enum;


struct Titi{Titi(){} Titi(int i){}};
struct Tutu
{
	int i;

	Tutu(){} 
	Tutu(int i, int j):i(i){}
private:
	Tutu(Tutu const&);
	Tutu& operator=(Tutu const&);
};

struct Toto
{
private:
	Toto(Toto const&);
	Toto& operator=(Toto const&);

public:
	enum
	{
		a,
		b,
		c,
		d
	};

	int i;
	std::vector<double> u;
	Tutu tutu;

	Toto():i(11111),tutu(1234, 1234){}

	Toto(int v):i(v),tutu(1234, 1234){}

	Toto(Tutu const& a, Tutu const& b):i(22222),tutu(1234, 1234){}

	Toto(int i, double d, StructForEnum_Enum e, std::string const& s):i(22222),tutu(1234, 1234)
	{
		std::cout << "i:" << i << " d:" << d << " e:" << e << " s:" << s << std::endl;
	}

	int func(int a) const
	{
		std::cout << a << std::endl;
		return a + 1;
	};

	int func2(Tutu const& a, StructForEnum_Enum b)
	{
		return 0;
	};

	//void funcP(Toto* ptr){ptr->i = 1;}
	//void funcPC(Toto const* ptr){std::cout << ptr->i << std::endl;}
	void funcR(Toto& ptr){ptr.i = 2;}
	void funcRC(Toto const& ptr){std::cout << ptr.i << std::endl;}
	//int funcP2(Toto* ptr){return ++ptr->i;}
	//int funcPC2(Toto const* ptr){return ptr->i;}
	int funcR2(Toto& ptr){return ++ptr.i;}
	int funcRC2(Toto const& ptr){return ptr.i;}

	int memberR(int i, double d, StructForEnum_Enum e, std::string const& s)
	{
		std::cout << "i:" << i << " d:" << d << " e:" << e << " s:" << s << std::endl;
		return 2;
	}

	void member(int i, double d, StructForEnum_Enum e, std::string const& s)
	{
		std::cout << "i:" << i << " d:" << d << " e:" << e << " s:" << s << std::endl;
	}
};

//void funcP(Toto* ptr){ptr->i = 1;}
//void funcPC(Toto const* ptr){std::cout << ptr->i << std::endl;}
void funcR(Toto& ptr){ptr.i = 2;}
void funcRC(Toto const& ptr){std::cout << ptr.i << std::endl;}
//int funcP2(Toto* ptr){return ++ptr->i;}
//int funcPC2(Toto const* ptr){return ptr->i;}
int funcR2(Toto& ptr){return ++ptr.i;}
int funcRC2(Toto const& ptr){return ptr.i;}

void funcA(Toto& ptr, StructForEnum_Enum e){ptr.i = 2;}
int funcB(Toto const& ptr, StructForEnum_Enum e){return ptr.i;}

int sqr(int u)
{
	return u * u;
}

int add(int u, int n)
{
	return int(u + n);
}

int limit(int val, int min, int max)
{
	return val < min? 
		min: 
		val > max? 
			max: 
			val;
}

void sqr_(int u)
{
	std::cout << (u * u) << std::endl;
}

void add_(int u, int n)
{
	std::cout << (u + n) << std::endl;
}

void limit_(int val, int min, int max)
{
	std::cout << 
		(val < min? 
			min: 
			val > max? 
				max: 
				val) << std::endl;
}

void tryEnum(StructForEnum_Enum){}


/*template<size_t I, typename... Args>
void displayArg(Args... args)
{
	using namespace Polua;
	std::cout << typeid(typename AtIndice<I, Args...>::Type).name() << std::endl;
};

struct OnTypeList
{
	template<typename... Args>
	struct A
	{
		template<typename T>
		struct B;

		template<std::size_t... Indices>
		struct B<Polua::indices<Indices...> >
		{
		};
	};

	template<typename... Args>
	static void displayTypes()
	{
		//displayTypesImpl<typename build_indices<sizeof...(Args)>::Type>::make();
		A<Args...>::B<Polua::indices<1,2, 3> >;
	}
};*/

void main()
try
{
	using namespace Polua;
	//std::cout << typeid(build_indices<4>::Type).name() << std::endl;
	//OnTypeList::displayTypes<int, Tutu, Titi, Toto, double, char, float>();

	Polua::State L;
	L.openlibs();

	Polua::Class<Toto>(L.state(), "Toto")
		.ctor()
		.ctor<int>()
		.ctor<Tutu, Tutu>()
		.ctor<int, double, StructForEnum::Enum, std::string>()
		.property("i", &Toto::i)
		.property("u", &Toto::u)
		.read_only("tutu", &Toto::tutu)
		.methode("func", &Toto::func)
		.methode("func2", &Toto::func2)
		//.methode("funcP", funcP)
		//.methode("funcPC", funcPC)
		.methode("funcR", funcR)
		.methode("funcRC", funcRC)
		//.methode("funcP2", funcP2)
		//.methode("funcPC2", funcPC2)
		.methode("funcR2", funcR2)
		.methode("funcRC2", funcRC2)
		//.methode("_funcP", &Toto::funcP)
		//.methode("_funcPC", &Toto::funcPC)
		.methode("_funcR", &Toto::funcR)
		.methode("_funcRC", &Toto::funcRC)
		//.methode("_funcP2", &Toto::funcP2)
		//.methode("_funcPC2", &Toto::funcPC2)
		.methode("_funcR2", &Toto::funcR2)
		.methode("_funcRC2", &Toto::funcRC2)
		.enumValue("a", Toto::a)
		.enumValue("b", Toto::b)
		.enumValue("c", Toto::c)
		.enumValue("d", Toto::d)
		.methode("memberR", &Toto::memberR)
		.methode("member", &Toto::member)
		.methode("funcA", funcA)
		.methode("funcB", funcB);

	
	typedef std::vector<double> DblVect;
	void (std::vector<double>::*resizePtr)(size_t n, const double& val) = &DblVect::resize;
	Polua::Class<std::vector<double> >(L.state(), "DoubleVect")
		.ctor()
		.methode("resize", resizePtr);

	Polua::regFunc(L.state(), "sqr",    &sqr);
	Polua::regFunc(L.state(), "add",    &add);
	Polua::regFunc(L.state(), "limit",  &limit);
	Polua::regFunc(L.state(), "sqr_",   &sqr_);
	Polua::regFunc(L.state(), "add_",   &add_);
	Polua::regFunc(L.state(), "limit_", &limit_);
	Polua::regFunc(L.state(), "tryEnum", &tryEnum);

	Polua::Class<Titi>(L.state(), "Titi").ctor<int>();
	Polua::Class<Tutu>(L.state(), "Tutu").ctor().ctor<int, int>().property("i", &Tutu::i);

	std::string const code =
		"function test(titi, tutu, val)\n"
		//"  titi = Titi(4)\n"
		//"  tutu = Tutu(48, 59)\n"
		"  toto = Toto(1, 1.5, 15, 'turlututu')\n"
		"  toto.memberR(1, 1.5, 15, 'turlututu')\n"
		"  print(Toto.a)\n"
		"  print(Toto.b)\n"
		"  print(Toto.c)\n"
		"  print(Toto.d)\n"
		"  print(toto.tutu.i)\n"
		"  print('i i i i i h b hjkfsb')\n"
		"  print(titi)\n"
		"  print(tutu)\n"
		"  print(val)\n"
		"  print('dfgmldkfgldfk')\n"
		"  print(sqr(4))\n"
		"  print(add(8, 9))\n"
		"  print(limit(18, 9, 29))\n"
		"  sqr_(5)\n"
		"  add_(15, 11)\n"
		"  limit_(30, 9, 27)\n"
		"  toto = Toto()\n"
		"  print(toto.i)\n"
		"  toto.i = 44444\n"
		"  print(toto.i)\n"
		"  print(toto.func(10000))\n"
		"  toto.u.resize(20, 0)\n"
		"  toto.u[3] = 55555\n"
		"  print(toto.u[3])\n"
		"  if 1 == 1 then\n"
		"    local vect = DoubleVect()\n"
		"    vect.resize(100, 0)\n"
		"    vect[45] = 66666\n"
		"    print(vect[45])\n"
		"    local tutu = Toto()\n"
		"    bug = tutu.u\n"
		"  end\n"
		"  print('table')\n"
		"  tab1 = {[0] = 'z', [1] = 'a', [5] = 'e', [2] = 'b', [3] = 'c', [4] = 'd', ['poutou']=48}\n"
		"  print()\n"
		"  for k, v in ipairs(tab1) do print(k, v) end\n"
		"  print()\n"
		"  print('vector')\n"
		"  for k, v in pairs(tab1) do print(k, v) end\n"
		"  print()\n"
		"  for k, v in ipairs(toto.u) do print(k, v) end\n"
		"  print('Test 2')\n"
		"  toto2 = Toto()\n"
		//"  print('funcP')\n"
		//"  toto.funcP()\n"
		//"  print('funcPC')\n"
		//"  toto.funcPC()\n"
		"  print('funcR')\n"
		"  toto.funcR()\n"
		"  print('funcRC')\n"
		"  toto.funcRC()\n"
		//"  print('funcP2')\n"
		//"  print(toto.funcP2())\n"
		//"  print('funcPC2')\n"
		//"  print(toto.funcPC2())\n"
		"  print('funcR2')\n"
		"  print(toto.funcR2())\n"
		"  print('funcRC2')\n"
		"  print(toto.funcRC2())\n"
		//"  print('_funcP')\n"
		//"  toto._funcP(toto2)\n"
		//"  print('_funcPC')\n"
		//"  toto._funcPC(toto2)\n"
		"  print('_funcR')\n"
		"  toto._funcR(toto2)\n"
		"  print('_funcRC')\n"
		"  toto._funcRC(toto2)\n"
		//"  print('_funcP2')\n"
		//"  print(toto._funcP2(toto2))\n"
		//"  print('_funcPC2')\n"
		//"  print(toto._funcPC2(toto2))\n"
		"  print('_funcR2')\n"
		"  print(toto._funcR2(toto2))\n"
		"  print('_funcRC2')\n"
		"  print(toto._funcRC2(toto2))\n"
		"  print(Toto(3))\n"
		"  print(Toto(Tutu(), Tutu()))\n"
		"end\n";
	Polua::throwOnError(L.state(), L.loadString(code, "code"));
	Polua::throwOnError(L.state(), lua_pcall(L.state(), 0, 0, 0));

	lua_getglobal(L.state(), "test");
	Polua::Caller caller(L.state());
	Titi titi(1);
	Tutu tutu(2, 3);
	Toto toto;
	caller.call(titi, 3, toto);
	lua_gc(L.state(), LUA_GCCOUNT, 0);
	system("pause");
}
catch(std::exception const& ex)
{
	std::cout << typeid(ex).name() << " " << ex.what() << std::endl;
	system("pause");
}