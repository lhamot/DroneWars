#ifndef __DRONEWARS_TEXT_GETTER__
#define __DRONEWARS_TEXT_GETTER__

#include "stdafx.h"

class TextGetter
{
	TextGetter(TextGetter const&);
	TextGetter& operator=(TextGetter const&);

	std::string lang_;
	std::unordered_map<std::string, std::string> translation_;

public:
	static TextGetter& GetInstance();

	TextGetter();
	~TextGetter();

	void setLang(std::string const& lang) {lang_ = lang;}

	std::string getLang() const {return lang_;}

	void loadModule(std::string const& modueName);

	std::string const& gettext(std::string const& message);
};

std::string const& gettext(std::string const& message);

#endif //__DRONEWARS_TEXT_GETTER__