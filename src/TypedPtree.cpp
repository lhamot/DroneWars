//
// Copyright (c) 2018 Loïc HAMOT
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "stdafx.h"
#include "TypedPtree.h"


std::ostream& operator<< (std::ostream& os, NilType)
{
	os << "nil";
	return os;
}

size_t cleanPtreeNilRec(TypedPtree& pt)
{
	size_t count = 0;
	for(auto iter = pt.begin(), end = pt.end(); iter != end;)
	{
		size_t const valueCount = cleanPtreeNilRec(iter->second);
		if(valueCount == 0)
			iter = pt.erase(iter);
		else
			++iter;
		count += valueCount;
	}
	boost::optional<Any> any = pt.get_value_optional<Any>();
	if(any && any->which())
		count += 1;
	return count;
}

void cleanPtreeNil(TypedPtree& pt)
{
	cleanPtreeNilRec(pt);
}

size_t countPtreeItem(TypedPtree const& pt)
{
	size_t count = 0;
	boost::optional<Any> any = pt.get_value_optional<Any>();
	if(any && any->which())
		count += 1;
	for(auto const& nvp : pt)
		count += countPtreeItem(nvp.second);
	return count;
}