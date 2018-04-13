//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "Tools.h"

#include <boost/locale/generator.hpp>

std::locale getLocal(std::string const& loc)
{
	boost::locale::generator gen;
	gen.add_messages_path(".");
	gen.add_messages_domain("DroneWars");
	return gen(loc);
}
