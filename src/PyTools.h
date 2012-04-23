#ifndef __PIXFIGHT_LVLTOOLS__
#define __PIXFIGHT_LVLTOOLS__

#include <iostream>
#include <fstream>
#pragma warning(push)
#pragma warning(disable: 4244 4100 4121 4127 4251)
#include <boost/python.hpp>
#pragma warning(pop)
#include <boost/exception/all.hpp>

namespace PyTools
{
struct PythonException : std::runtime_error
{
	PythonException(std::string const& message): std::runtime_error(message) {}
};


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
}

class PythonEngine
{
	boost::python::object objStringIO;

public:
	PythonEngine()
	{
		using namespace boost::python;

		Py_Initialize();

		initPythonStdErr();
	}

	~PythonEngine()
	{
		Py_Finalize();
	}
};

}

#endif //__PIXFIGHT_LVLTOOLS__