#include "stdafx.h"
#include "Logger.h"
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <boost/thread.hpp>

using namespace boost::posix_time;

std::string Logger::fileName_ = "logfile.txt";


Logger::Logger()
{
	file_.open(fileName_.c_str(), std::ios_base::app);
	file_ << second_clock::local_time() << " ";
}

Logger::~Logger()
{
	file_ << std::endl;
}

void Logger::SetFile(const char* fileName)
{
	fileName_ = fileName;
}

std::ostream& Logger::getStream()
{
	return file_;
}
