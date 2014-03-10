//! @file
//! @author Lo�c HAMOT

#ifndef __BTA_TOOLS__
#define __BTA_TOOLS__

#include "stdafx.h"
#include <string>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/iterator_range.hpp>

//! Obtient une local pour la culture demand�e
//! @todo: Si le nombre de trad pas seconde augmente, il faudra surement
//!   pr�parer les locale et les r�utiliser.
std::locale getLocal(std::string const& loc);


//! @brief Recherche un iterateur dans une map et le retourne
//! @throw std::logic_error si la cl� n'existe pas
template<typename M>
auto mapFind(M& map, typename M::key_type key) -> decltype(map.find(key))
{
	auto iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}


//! Recherche dans une map le prochain iterateur qui n'as pas la m�me cl�
template<typename T, typename I>
typename T::iterator
nextNot(T& map, I const& iter)
{
	I res = iter;
	auto mapEnd = map.end();
	while((res != mapEnd) && (res->first == iter->first))
		++res;
	return res;
}


//! @brief Supprime de la map tout les �l�ments correspondants a une condition
template<typename M, typename F>
void map_remove_erase_if(M& map, F const& func)
{
	auto iter = map.begin();
	while(iter != map.end())
	{
		if(func(*iter))
		{
			auto prev = iter;
			++iter;
			map.erase(prev);
		}
		else
			++iter;
	}
}


//! Traits pour simuler un static if
template<bool B>
struct StaticIf;


//! Specialisation pour le cas ou la condition est vraie
template<>
struct StaticIf<true>
{
	//! Puisque la condition est vrai, on excecute la function
	template<typename F>
	static void exec(F f)
	{
		f();
	}
};

//! Specialisation pour le cas ou la condition est fausse
template<>
struct StaticIf<false>
{
	//! Puisque la condition est fausse, on ne fait rien
	template<typename F>
	static void exec(F)
	{
	}
};


//! Appelle la fonction f, si B est true, pour simuler un static if
template<bool B, typename F>
void staticIf(F f)
{
	StaticIf<B>::exec(f);
}

template<typename O, typename I>
O lexicalCast(I in, char const* const filename, int line)
{
	try
	{
		return boost::lexical_cast<O>(in);
	}
	catch(boost::bad_lexical_cast& blc)
	{
		boost::throw_exception(
		  boost::enable_error_info(blc) <<
		  boost::throw_file(filename) <<
		  boost::throw_line(line));
	}
}

#define LEXICAL_CAST(O, in) \
	lexicalCast<O>(in, __FILE__, __LINE__)

template<typename M>
auto mapFind(M& map, typename M::key_type key, char const* const filename, int line) -> decltype(map.find(key))
{
	auto iter = map.find(key);
	if(iter == map.end())
		boost::throw_exception(
		  boost::enable_error_info(std::logic_error("Can't find item")) <<
		  boost::throw_file(filename) <<
		  boost::throw_line(line));
	return iter;
}

#define MAP_FIND(map, key) \
	mapFind(map, key, __FILE__, __LINE__)

#endif //__BTA_TOOLS__
