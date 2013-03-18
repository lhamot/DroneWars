#ifndef __DRONEWARS_LOGGER__
#define __DRONEWARS_LOGGER__

#include <iosfwd>
#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>


#define CATCH_LOG_RETHROW(LOGGER) \
	catch(std::exception &ex) \
	{ \
		LOG4CPLUS_ERROR(LOGGER, boost::diagnostic_information(ex)); \
		throw; \
	} \
	catch(...) \
	{ \
		LOG4CPLUS_ERROR(LOGGER, boost::current_exception_diagnostic_information()); \
		throw; \
	}

#define CATCH_LOG_EXCEPTION(LOGGER) \
	catch(std::exception &ex) \
	{ \
		LOG4CPLUS_ERROR(LOGGER, boost::diagnostic_information(ex)); \
	} \
	catch(...) \
	{ \
		LOG4CPLUS_ERROR(LOGGER, boost::current_exception_diagnostic_information()); \
	}


#endif //__DRONEWARS_LOGGER__