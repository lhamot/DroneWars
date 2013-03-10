#ifndef __DRONEWARS_LOGGER__
#define __DRONEWARS_LOGGER__

#include <iosfwd>
#include <string>

#define LOG_ERROR Logger().getStream() << __FILE__ << " " << __LINE__ << std::endl
#define LOG_VAR(varName) LOG_ERROR << #varName << " : " << varName

#define CATCH_LOG_RETHROW \
	catch(std::exception &ex) \
	{ \
		LOG_ERROR << boost::diagnostic_information(ex); \
		throw; \
	} \
	catch(...) \
	{ \
		LOG_ERROR << boost::current_exception_diagnostic_information(); \
		throw; \
	}

#define CATCH_LOG_EXCEPTION \
	catch(std::exception &ex) \
	{ \
		LOG_ERROR << boost::diagnostic_information(ex); \
	} \
	catch(...) \
	{ \
		LOG_ERROR << boost::current_exception_diagnostic_information(); \
	}


class Logger
{
	std::ofstream file_;
	static std::string fileName_;

public:

	Logger();
	~Logger();

	static void SetFile(const char* fileName);

	std::ostream& getStream();
};


#endif __DRONEWARS_LOGGER__