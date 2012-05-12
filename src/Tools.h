#ifndef __BTA_TOOLS__
#define __BTA_TOOLS__

#include <iostream>

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
-> decltype(std::make_pair(make_zip_iterator(make_tuple(c1.begin(), c2.begin())),
                           make_zip_iterator(make_tuple(c1.end(), c2.end()))))
{
	auto begin = make_zip_iterator(make_tuple(c1.begin(), c2.begin()));
	auto end = make_zip_iterator(make_tuple(c1.end(), c2.end()));
	return make_pair(begin, end);
}


#define TRACE std::cout << __FILE__ << " " << __LINE__ << std::endl;

#endif //__BTA_TOOLS__