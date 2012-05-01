#include "MainForm.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <Wt/WStackedWidget>
#pragma warning(pop)

#include "bit_them_allWT.h"
#include "OutPage.h"


using namespace Wt;

MainForm::MainForm(
	WContainerWidget* parent, Engine& engine):
	WContainerWidget(parent),
	engine_(engine)
{
	//WStackedWidget *stacked = new WStackedWidget(this);
	//addWidget(stacked);
	//stacked->addWidget(createInPage(stacked));
	//stacked->addWidget(createOutPage(stacked));
	OutPage* outPage = nullptr;
	addWidget(outPage = createOutPage(this));
	outPage->onPlayerLogin = [&]
	(Player::ID& pid)
	{
		//this->onPlayerLogin();
		removeWidget(widget(0));
		addWidget(new bit_them_allWT(this, engine_, pid));
	};
}


void MainForm::onPlayerLogin()
{
	//removeWidget(widget(0));
	//addWidget(createInPage(this));
}


OutPage* MainForm::createOutPage(Wt::WContainerWidget* parent)
{
	return new OutPage(parent, engine_);
}

/*
bit_them_allWT* MainForm::createInPage(Wt::WContainerWidget* parent)
{
	return new bit_them_allWT(parent, engine_);
}*/
