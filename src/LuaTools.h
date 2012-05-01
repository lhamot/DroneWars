#ifndef __LUA_TOOLS__
#define __LUA_TOOLS__

#include <string>
#include <sstream>
#include <luabind\luabind.hpp>

/*extern "C"
{
#include "lua.h"
}*/


namespace LuaTools
{

struct ScriptException : std::runtime_error
{
	ScriptException(std::string const& message): std::runtime_error(message) {}
};

/*
inline void initPythonStdErr()
{
	using namespace boost::python;

	// Import cStringIO module
	object modStringIO = import("cStringIO");

	// get StringIO constructor
	object classStringIO = modStringIO.attr("StringIO");

	// Construct cStringIO object
	object objStringIO = classStringIO();

	// try assigning this object to sys.stderr
	object sysModule = import("sys");

	sysModule.attr("stderr") = objStringIO;
}

inline std::string getPyStdErr()
{
	using namespace boost::python;
	object sysModule = import("sys");
	object stdErr = sysModule.attr("stderr");
	std::string message = boost::python::extract<std::string>(stdErr.attr("getvalue")());
	initPythonStdErr();
	//std::cout << "stderr : " << message << std::endl;
	return message;
}*/

class LuaEngine
{
	lua_State* L;

	static int add_file_and_line(lua_State* L)
	{
		 lua_Debug d;
		 lua_getstack(L, 1, &d);
		 lua_getinfo(L, "Sln", &d);
		 char const* err = lua_tostring(L, -1);
		 lua_pop(L, 1);
		 std::stringstream msg;
		 msg << d.short_src << ":" << d.currentline;

		 if (d.name != 0)
		 {
				msg << "(" << d.namewhat << " " << d.name << ")";
		 }
		 msg << " " << err?std::string(err):"";
		 lua_pushstring(L, msg.str().c_str());
		 return 1;
	}

	static int  panicf(lua_State* L)
	{
		//add_file_and_line(L);
		//std::string message = lua_tostring(L, -1);
		luabind::object error_msg(luabind::from_stack(L, -1));
		std::stringstream ss;
		ss << error_msg;
		throw ScriptException(ss.str());
	}

public:
	LuaEngine(): L(lua_open())
	{
		lua_atpanic(L, &LuaEngine::panicf);
		luabind::set_pcall_callback(add_file_and_line);
	}

	~LuaEngine()
	{
		lua_close(L);
	}

	lua_State* state() {return L;}
};

}

#endif //__LUA_TOOLS__