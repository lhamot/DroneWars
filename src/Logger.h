//! @file
//! @author Loïc HAMOT

#ifndef __DRONEWARS_LOGGER__
#define __DRONEWARS_LOGGER__

typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> Logger;

BOOST_LOG_GLOBAL_LOGGER(logger, Logger)

//! catch une exception, la log et la relance
#define CATCH_LOG_RETHROW(LOGGER) \
	catch(std::exception &ex) \
	{ \
		DW_LOG_ERROR << boost::diagnostic_information(ex); \
		throw; \
	} \
	catch(...) \
	{ \
		DW_LOG_ERROR << boost::current_exception_diagnostic_information(); \
		throw; \
	}

//! catch une exception, la log et ne la relance pas
#define CATCH_LOG_EXCEPTION(LOGGER) \
	catch(std::exception &ex) \
	{ \
		DW_LOG_ERROR << boost::diagnostic_information(ex); \
	} \
	catch(...) \
	{ \
		DW_LOG_ERROR << boost::current_exception_diagnostic_information(); \
	}

namespace LoggerKW
{
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int);
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string);
BOOST_LOG_ATTRIBUTE_KEYWORD(function, "Function", std::string);
}

#define DW_LOG_SEV(level) \
	BOOST_LOG_WITH_PARAMS(logger::get(), (boost::log::keywords::severity = level)) \
	    << boost::log::add_value("Line", __LINE__) \
	    << boost::log::add_value("File", __FILE__) \
	    << boost::log::add_value("Function", __func__)

#define DW_LOG_TRACE DW_LOG_SEV(boost::log::trivial::trace)
#define DW_LOG_DEBUG DW_LOG_SEV(boost::log::trivial::debug)
#define DW_LOG_INFO DW_LOG_SEV(boost::log::trivial::info)
#define DW_LOG_WARNING DW_LOG_SEV(boost::log::trivial::warning)
#define DW_LOG_ERROR DW_LOG_SEV(boost::log::trivial::error)
#define DW_LOG_FATAL DW_LOG_SEV(boost::log::trivial::fatal)

void initLogger(std::string const& filename);

#endif //__DRONEWARS_LOGGER__