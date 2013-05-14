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


//! déserialization d'un std::unordered_map
template<class Archive, class Type, class Key, class Hash, class Compare, class Allocator>
inline void load(
  Archive& ar,
  std::unordered_map<Key, Type, Hash, Compare, Allocator>& t,
  const unsigned int /* file_version */
)
{
	boost::serialization::stl::load_collection <
	Archive,
	std::unordered_map<Key, Type, Hash, Compare, Allocator>,
	boost::serialization::stl::archive_input_map <
	Archive, std::unordered_map<Key, Type, Hash, Compare, Allocator> > ,
	boost::serialization::stl::no_reserve_imp < std::unordered_map <
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
	boost::serialization::stl::load_collection <
	Archive,
	std::unordered_multimap<Key, Type, Hash, Compare, Allocator>,
	boost::serialization::stl::archive_input_map <
	Archive, std::unordered_multimap<Key, Type, Hash, Compare, Allocator>
	> ,
	boost::serialization::stl::no_reserve_imp <
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
