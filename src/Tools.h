#ifndef __BTA_TOOLS__
#define __BTA_TOOLS__

#include "stdafx.h"
//#include <iostream>
//#include <utility>
#include <string>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/iterator_range.hpp>


inline std::string escape(std::string const& code)
{
	std::string escaped;
	escaped.reserve(code.size() * 2);
	for(char c: code)
	{
		switch(c)
		{
		case '\"': escaped += "\\\""; break;
		case '\'': escaped += "\\\'"; break;
		case '\\': escaped += "\\\\"; break;
		case '\n': break;
		default: escaped.push_back(c);
		}
	}
	return escaped;
};


template<typename M>
typename M::const_iterator
mapFind(M const& map, typename M::key_type key)
{
	typename M::const_iterator iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}

template<typename M>
typename M::iterator
mapFind(M& map, typename M::key_type key)
{
	typename M::iterator iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}

template<typename T, typename I>
typename T::iterator
nextNot(T& map, I const& iter)
{
	I res = iter;
	while((res != map.end()) && (res->first == iter->first))
		++res;
	return res;
}


template<typename C1, typename C2>
auto make_zip_range(C1 const& c1, C2 const& c2)
-> decltype(boost::make_iterator_range(boost::make_zip_iterator(boost::make_tuple(c1.begin(), c2.begin())),
                                       boost::make_zip_iterator(boost::make_tuple(c1.end(), c2.end()))))
{
	auto begin = boost::make_zip_iterator(boost::make_tuple(c1.begin(), c2.begin()));
	auto end = boost::make_zip_iterator(boost::make_tuple(c1.end(), c2.end()));
	return boost::make_iterator_range(begin, end);
}

template<typename M, typename F>
void map_remove_erase_if(M& map, F const& func)
{
	auto iter = map.begin();
	while(iter != map.end())
	{
		if(func(*iter))
			map.erase(iter++);
		else
			++iter;
	}
}


#define TRACE std::cout << __FILE__ << " " << __LINE__ << std::endl;

#endif //__BTA_TOOLS__
