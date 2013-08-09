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
