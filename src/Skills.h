#ifndef __NDW_SKILLS__
#define __NDW_SKILLS__

#include "stdafx.h"

struct Skill
{
	std::string name;
	static size_t const Count = 11;
	static Skill const List[Count];
};


#endif //__NDW_SKILLS__