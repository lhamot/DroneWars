#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4512 4505 4275)
#include <boost/exception/all.hpp>
//#include <QtGui/QApplication>
#include <Wt/WApplication>
#include <Wt/WMessageBox>
#pragma warning(pop)

#include "Engine.h"
#include "MainForm.h"

using namespace Wt;

class BTAApplication : public WApplication
{
public:
	BTAApplication(const Wt::WEnvironment& env, Engine& engine): WApplication(env)
	{
		require("CodeMirror/lib/codemirror.js");
		require("CodeMirror/mode/lua/lua.js");
		useStyleSheet("CodeMirror/theme/neat.css");
		useStyleSheet("CodeMirror/lib/codemirror.css");
		
		/*doJavaScript(
			"var editor = CodeMirror.fromTextArea('code', {"
      "height: \"350px\","
      "parserfile: \"http://codemirror.net/mode/contrib/lua/js/parselua.js\","
      "stylesheet: \"http://codemirror.net/mode/lua/css/luacolors.css\","
      "path: \"http://codemirror.net/js/\""
      "});\""
	  );*/
		root()->addWidget(new MainForm(root(), engine));
		//doJavaScript("var myCodeMirror = CodeMirror(document.body);");
		/*doJavaScript(
			"var editor = CodeMirror.fromTextArea(document.getElementById(\"Fleet\"), {"
					"tabMode: \"indent\","
					"matchBrackets: true,"
					"theme: \"neat\""
				"});"
		);*/
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
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env, Engine& engine)
{
  WApplication *apply = new BTAApplication(env, engine);
	//apply->useStyleSheet("format.css");
	//apply->messageResourceBundle().use("Rybosome");
	//apply->setLocale("fr");
	return apply;
}

int main(int argc, char* argv[])
try
{
	srand(static_cast<unsigned int>(time(NULL)));

	Engine engine;
	return Wt::WRun(argc, argv, [&]
		(const Wt::WEnvironment& env){return createApplication(env, engine);
	});

	//BTAApplication a(argc, argv);
	//bit_them_all view(engine_);
	//view.show();
	//return a.exec();
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
	WMessageBox::show("Error", "Error <unknown>", Ok);
}
