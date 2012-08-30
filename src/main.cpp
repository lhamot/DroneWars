#include "stdafx.h"

#include "Engine.h"
#include "bit_them_allWT.h"
#include "OutPage.h"
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>


using namespace Wt;
using namespace std;

class BTAApplication : public WApplication
{
public:
	BTAApplication(const Wt::WEnvironment& env, Engine& engine):
		WApplication(env),
		engine_(engine),
		playerID_(Player::NoId)
	{
		require("CodeMirror/lib/codemirror.js");
		require("CodeMirror/mode/lua/lua.js");
		useStyleSheet("format.css");

		internalPathChanged().connect(this, &BTAApplication::handleInternalPath);

		setInternalPath("", true);
	}

private:
	void onPlayerLogin(Player::ID pid)
	{
		playerID_ = pid;
		setInternalPath("/ingame", true);
	}

	void handleInternalPath(const std::string& internalPath)
	{
		if(internalPath == "/ingame" && playerID_ != Player::NoId)
		{
			root()->removeWidget(root()->widget(0));
			root()->addWidget(new bit_them_allWT(root(), engine_, playerID_));
		}
		else
		{
			setInternalPath("", false);
			playerID_ = Player::NoId;
			if(root()->count() > 0)
				root()->removeWidget(root()->widget(0));
			OutPage* outPage = new OutPage(root(), engine_);
			root()->addWidget(outPage);
			outPage->onPlayerLogin = std::bind(&BTAApplication::onPlayerLogin, this, placeholders::_1);
		}
	}

	void notify(WEvent& event)
	{
		try
		{
			return WApplication::notify(event);
		}
		catch(boost::exception& e)
		{
			WMessageBox::show("Error", boost::diagnostic_information(e).c_str(), Ok);
		}
		catch(std::exception& e)
		{
			WMessageBox::show("Error", boost::diagnostic_information(e).c_str(), Ok);
		}
		catch(...)
		{
			WMessageBox::show("Error", boost::current_exception_diagnostic_information().c_str(), Ok);
			//QMessageBox::critical(0, "Error", "Error <unknown> sending event %s to object %s (%s)",
			//       typeid(*event).name(), qPrintable(receiver->objectName()),
			//       typeid(*receiver).name());
		}

		// qFatal aborts, so this isn't really necessary
		// but you might continue if you use a different logging lib
		//return false;
	}

	Engine& engine_;
	Player::ID playerID_;
};

Wt::WApplication* createApplication(const Wt::WEnvironment& env, Engine& engine)
{
	WApplication* apply = new BTAApplication(env, engine);
	//apply->useStyleSheet("format.css");
	//apply->messageResourceBundle().use("Rybosome");
	//apply->setLocale("fr");
	return apply;
}

int main(int argc, char* argv[])
try
{
	{
		//Pour que WT convertise correctement mes string en WString
		auto uft8CodeCvt = new boost::program_options::detail::utf8_codecvt_facet;
		std::locale::global(std::locale(std::locale(), uft8CodeCvt));
	}

	srand(static_cast<unsigned int>(time(NULL)));

	putenv("LANG=fr_FR");
#ifdef _WIN32
	printf("Locale is: %s\n", setlocale(LC_ALL, "fr"));
#else
	printf("Locale is: %s\n", setlocale(LC_ALL, "fr_FR.UTF8"));
#endif
	bindtextdomain("DroneWars", "./");
	textdomain("DroneWars");

	Engine engine;
	return Wt::WRun(argc, argv, [&]
	                (const Wt::WEnvironment & env)
	{
		return createApplication(env, engine);
	});
	return 0;
}
catch(boost::exception& e)
{
	std::cout << boost::diagnostic_information(e) << std::endl;
	return 1;
}
catch(std::exception& e)
{
	std::cout << boost::diagnostic_information(e) << std::endl;
	return 1;
}
catch(...)
{
	std::cout << "Error <unknown>" << std::endl;
	return 1;
}
