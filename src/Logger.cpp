//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "Logger.h"

#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

BOOST_LOG_GLOBAL_LOGGER_INIT(logger, Logger)
{
	return Logger();
}

void initLogger(std::string const& filename)
{
	boost::log::register_simple_filter_factory<boost::log::trivial::severity_level, char>("Severity");
	boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
	std::ifstream logConfig(filename);
	boost::log::init_from_stream(logConfig);
	boost::log::add_common_attributes();
}
