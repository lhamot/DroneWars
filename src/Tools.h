#ifndef __BTA_TOOLS__
#define __BTA_TOOLS__

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

#endif //__BTA_TOOLS__