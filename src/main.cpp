#pragma warning(push)
#pragma warning(disable:4127 4251 4231)
#include <boost/exception/all.hpp>
#include <QtGui/QApplication>
#include <QMessageBox>
#pragma warning(pop)

#include "Engine.h"
#include "bit_them_all.h"

class BTAApplication : public QApplication
{
public:
	BTAApplication(int argc, char* argv[]): QApplication(argc, argv)
	{
	}

	bool notify(QObject* receiver, QEvent* event)
	{
		try
		{
			return QApplication::notify(receiver, event);
		}
		catch(boost::exception& e)
		{
			QMessageBox::critical(0, "Error", boost::diagnostic_information(e).c_str());
		}
		catch(std::exception& e)
		{
			QMessageBox::critical(0, "Error", boost::diagnostic_information(e).c_str());
		}
		catch(...)
		{
			QMessageBox::critical(0, "Error", boost::current_exception_diagnostic_information().c_str());
			//QMessageBox::critical(0, "Error", "Error <unknown> sending event %s to object %s (%s)",
			//       typeid(*event).name(), qPrintable(receiver->objectName()),
			//       typeid(*receiver).name());
		}

		// qFatal aborts, so this isn't really necessary
		// but you might continue if you use a different logging lib
		return false;
	}
};

int main(int argc, char* argv[])
try
{
	srand(static_cast<unsigned int>(time(NULL)));

	BTAApplication a(argc, argv);
	Engine engine_;
	bit_them_all view(engine_);
	view.show();
	return a.exec();
}
catch(boost::exception& e)
{
	QMessageBox::critical(0, "Error", boost::diagnostic_information(e).c_str());
}
catch(std::exception& e)
{
	QMessageBox::critical(0, "Error", boost::diagnostic_information(e).c_str());
}
catch(...)
{
	QMessageBox::critical(0, "Error", "Error <unknown>");
}
