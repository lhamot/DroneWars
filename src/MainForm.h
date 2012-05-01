#ifndef __BTA_MAIN_FORM__
#define __BTA_MAIN_FORM__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4512 4275 4505)
#include <Wt/WContainerWidget>
#pragma warning(pop)

#include "Engine.h"

class OutPage;
class bit_them_allWT;

class MainForm : public Wt::WContainerWidget
{

public:
	MainForm(Wt::WContainerWidget* parent, Engine& engine);

private:
	OutPage* createOutPage(Wt::WContainerWidget*);
	//bit_them_allWT* createInPage(Wt::WContainerWidget*);
	void onPlayerLogin();

	bit_them_allWT *inPage_;
	OutPage *outPage_;
	Engine& engine_;
};

#endif //__BTA_MAIN_FORM__