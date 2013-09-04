#ifndef __NDW_TYPED_PTREE__
#define __NDW_TYPED_PTREE__

#pragma warning(push)
#pragma warning(disable: 4345)
#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>
#pragma warning(pop)


struct NilType {}; //! Object par defaut du AnyTypeList
//! Operateur de sortie dans un flux du NilType
std::ostream& operator<< (std::ostream& os, NilType);
//! Operateur d'�galit� du NilType
inline bool operator == (NilType, NilType)
{
	return true;
}

//! Liste des types(dans l'ordre) contenue dans un Any
enum class AnyTypeList {Nil, Boolean, Integer, Decimal, Text};
//! Object qui peut etre different types a l'excecution
typedef boost::variant<NilType, bool, ptrdiff_t, double, std::string> Any;
//! Arbre assosiant un nom a un Any
typedef boost::property_tree::basic_ptree<std::string, Any> TypedPtree;
//! Supprime les �l�ment inutile d'un TypedPtree (c-a-d les NilType)
void cleanPtreeNil(TypedPtree& pt);
//! Compte les �l�ments dans un TypedPtree (de mani�re r�cursive)
size_t countPtreeItem(TypedPtree const& pt);

namespace boost
{
namespace serialization
{
//! Serialize un NilType (Ne fait rien)
template<class Archive>
void serialize(Archive&,// ar,
               NilType&,// value,
               const unsigned int// version
              )
{
}

} // namespace serialization
} // namespace boost

#endif //__NDW_TYPED_PTREE__