//! @file
//! @author Loïc HAMOT

#ifndef __BTA_TOOLS__
#define __BTA_TOOLS__

#include "stdafx.h"
#include <string>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/iterator_range.hpp>

//! Obtient une local pour la culture demandée
//! @todo: Si le nombre de trad pas seconde augmente, il faudra surement
//!   préparer les locale et les réutiliser.
std::locale getLocal(std::string const& loc);


//! @brief Recherche un iterateur dans une map et le retourne
//! @throw std::logic_error si la clé n'existe pas
template<typename M>
auto mapFind(M& map, typename M::key_type key) -> decltype(map.find(key))
{
	auto iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}


//! Recherche dans une map le prochain iterateur qui n'as pas la mème clé
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


//! Crée un zip_iterator à partir de deux iterateurs
template<typename I1, typename I2>
auto make_zip_iterator(I1 const& iter1, I2 const& iter2)
-> decltype(boost::make_zip_iterator(boost::make_tuple(iter1, iter2)))
{
	return boost::make_zip_iterator(boost::make_tuple(iter1, iter2));
}


//! Crée un range pour parcourir deux conteneurs à la fois (voir zip en python)
template<typename C1, typename C2>
auto make_zip_range(C1 const& c1, C2 const& c2)
-> decltype(boost::make_iterator_range(
              make_zip_iterator(c1.begin(), c2.begin()),
              make_zip_iterator(c1.end(), c2.end())))
{
	auto begin = make_zip_iterator(c1.begin(), c2.begin());
	auto end = make_zip_iterator(c1.end(), c2.end());
	return boost::make_iterator_range(begin, end);
}



//! @brief Supprime de la map tout les éléments correspondants a une condition
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


#endif //__BTA_TOOLS__
