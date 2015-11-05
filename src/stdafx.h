//! @file
//! @author Loïc HAMOT

//std
#include <algorithm>
#include <ctime>
#include <functional>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>
#include <sys/stat.h>
#include <set>
#include <tuple>
#include <vector>
#include <utility>
#include <unordered_map>

#include "SafeInt3.hpp"

#pragma warning(push)
#pragma warning(disable:4244 4310 4100 4512 4127 4456 4458 4459 4706 4702 4503 4345 6011)
#include <boost/array.hpp>
#include <boost/exception/all.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem.hpp>
#include <boost/geometry/geometries/adapted/boost_array.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/combine.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/variant.hpp>
#include <boost/locale.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#pragma warning(pop)

#define NOMINMAX