//! @file
//! @author Loïc HAMOT

#ifndef  BOOST_SERIALIZATION_UNORDERED_MAP_HPP
#define BOOST_SERIALIZATION_UNORDERED_MAP_HPP

#include <unordered_map>

#include <boost/config.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
namespace serialization
{


//! serialization d'un std::unordered_map
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator>
inline void save(
  Archive& ar,
  const std::unordered_map<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int /* file_version */
)
{
	boost::serialization::stl::save_collection <
	Archive,
	std::unordered_map<Key, Type, Hash, Compare, Allocator>
	> (ar, t);
}

template<class Container>
class no_reserve_imp
{
public:
	void operator()(Container& /* s */, std::size_t /* count */) const {}
};

// map input
template<class Archive, class Container>
struct archive_input_map
{
	inline BOOST_DEDUCED_TYPENAME Container::iterator
	operator()(
	  Archive& ar,
	  Container& s,
	  const unsigned int v,
	  BOOST_DEDUCED_TYPENAME Container::iterator hint
	)
	{
		typedef BOOST_DEDUCED_TYPENAME Container::value_type type;
		detail::stack_construct<Archive, type> t(ar, v);
		// borland fails silently w/o full namespace
		ar >> boost::serialization::make_nvp("item", t.reference());
		BOOST_DEDUCED_TYPENAME Container::iterator result =
		  s.insert(hint, t.reference());
		// note: the following presumes that the map::value_type was NOT tracked
		// in the archive.  This is the usual case, but here there is no way
		// to determine that.
		ar.reset_object_address(
		  &(result->second),
		  &t.reference().second
		);
		return result;
	}
};

template<class Archive, class Container, class InputFunction, class R>
inline void load_collection(Archive& ar, Container& s)
{
	s.clear();
	collection_size_type count;
	const boost::archive::library_version_type library_version(
	  ar.get_library_version()
	);
	// retrieve number of elements
	item_version_type item_version(0);
	ar >> BOOST_SERIALIZATION_NVP(count);
	if(boost::archive::library_version_type(3) < library_version)
	{
		ar >> BOOST_SERIALIZATION_NVP(item_version);
	}

	R rx;
	rx(s, count);
	InputFunction ifunc;
	BOOST_DEDUCED_TYPENAME Container::iterator hint;
	hint = s.begin();
	while(count-- > 0)
	{
		hint = ifunc(ar, s, item_version, hint);
	}
}

//! déserialization d'un std::unordered_map
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator>
inline void load(
  Archive& ar,
  std::unordered_map<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int /* file_version */
)
{
	load_collection <
	Archive,
	std::unordered_map<Key, Type, Hash, Compare, Allocator>,
	archive_input_map <
	Archive, std::unordered_map<Key, Type, Hash, Compare, Allocator> > ,
	no_reserve_imp < std::unordered_map <
	Key, Type, Hash, Compare, Allocator
	>
	>
	> (ar, t);
}


//! serialize ou déserialize d'un std::unordered_map
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator>
inline void serialize(
  Archive& ar,
  std::unordered_map<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int file_version
)
{
	boost::serialization::split_free(ar, t, file_version);
}


//! serialize un std::unordered_multimap
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator >
inline void save(
  Archive& ar,
  const std::unordered_multimap<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int /* file_version */
)
{
	boost::serialization::stl::save_collection <
	Archive,
	std::unordered_multimap<Key, Type, Hash, Compare, Allocator>
	> (ar, t);
}


//! déserialization d'un std::unordered_multimap
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator >
inline void load(
  Archive& ar,
  std::unordered_multimap<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int /* file_version */
)
{
	load_collection <
	Archive,
	std::unordered_multimap<Key, Type, Hash, Compare, Allocator>,
	archive_input_map <
	Archive, std::unordered_multimap<Key, Type, Hash, Compare, Allocator>
	> ,
	no_reserve_imp <
	std::unordered_multimap<Key, Type, Hash, Compare, Allocator>
	>
	> (ar, t);
}


//! serialize ou déserialize d'un std::unordered_multimap
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator >
inline void serialize(
  Archive& ar,
  std::unordered_multimap<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int file_version
)
{
	boost::serialization::split_free(ar, t, file_version);
}
} // serialization
} // namespace boost

#endif // BOOST_SERIALIZATION_UNORDERED_MAP_HPP
