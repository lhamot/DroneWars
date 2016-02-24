//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "NameGen.h"


std::string nameGen(size_t length)
{
	if(length == -1)
		length = (rand() % 3) + 3;
	char const* const vowels[] =
	{
		"a", "e", "i", "o", "u", "ou", "in", "on",
		"ain", "ein", "eu", "an", "en", "y"
	}; //14
	char const* const consonants[] =
	{
		"b", "c", "d", "f", "g", "j", "k", "l", "m", "n", "p",
		"q", "r", "s", "t", "v", "w", "x", "z", "ch", "qu"
	}; //21
	size_t voyCount = sizeof(vowels) / sizeof(char*);
	size_t conCount = sizeof(consonants) / sizeof(char*);
	char const* const vMag = "AEIOUY";
	char const* const cMag = "BCDFGHJKLMNPQRSTVWXZ";
	std::string name;
	size_t alt = rand() % 2;
	if(alt == 1)
	{
		name += cMag[(rand() % strlen(cMag))];
		alt = 0;
	}
	else
	{
		name += vMag[(rand() % strlen(vMag))];
		alt = 1;
	}
	--length;
	for(size_t i = 0; i < length; i++)
	{
		if(i == length - 2)
			--voyCount;
		if(i == length - 1)
			--conCount;
		if(alt == 1)
		{
			name += consonants[rand() % conCount];
			alt = 0;
		}
		else
		{
			name += vowels[rand() % voyCount];
			alt = 1;
		}
	}
	return name;
}