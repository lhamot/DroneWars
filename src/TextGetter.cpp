#include "stdafx.h"

#include "TextGetter.h"


boost::mutex mut;

TextGetter& TextGetter::GetInstance()
{
	boost::unique_lock<boost::mutex> lock(mut);
	static TextGetter* instance = nullptr;
	if(instance == nullptr)
		instance = new TextGetter();
	return *instance;
}

TextGetter::TextGetter() {}

TextGetter::~TextGetter() {}


void TextGetter::loadModule(std::string const& modueName)
{
	std::string const filename = lang_ + '/' + modueName + ".po";
	std::ifstream file(filename);
	if(file.is_open() == false)
		return;
	std::string line;
	while(std::getline(file, line))
	{
		if(line.find("msgid ") == 0)
		{
			std::string key = line.substr(7, line.size() - 8);
			if(std::getline(file, line) == false)
				return;
			if(line.find("msgstr ") != 0)
				return;
			std::string value = line.substr(8, line.size() - 9);
			translation_.insert(std::make_pair(key, value));
		}
	}
}

std::string const& TextGetter::gettext(std::string const& message)
{
	auto iter = translation_.find(message);
	if(iter == translation_.end())
		return message;
	else
		return iter->second;
}

std::string const& gettext(std::string const& message)
{
	return TextGetter::GetInstance().gettext(message);
}
