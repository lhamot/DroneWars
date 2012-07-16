#include "stdafx.h"
#include "OutPage.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WDefaultLayout>
#include <Wt/WStackedWidget>
#include <Wt/WMenu>
#include <Wt/WBreak>
#include <Wt/WMessageBox>
#include <Wt/WTable>
#include <Wt/WLengthValidator>
#pragma warning(pop)


using namespace Wt;

Wt::WContainerWidget* OutPage::createHomePage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* homePage = new WContainerWidget(parent);
	Wt::WTable* table = new Wt::WTable(homePage);

	table->elementAt(0, 0)->addWidget(new WText("login :", parent));
	table->elementAt(0, 1)->addWidget(loginEdit_ = new WLineEdit(parent));
	loginEdit_->setValidator(new WLengthValidator(0, MaxStringSize, loginEdit_));
	//table->elementAt(0, 0)->->addWidget(new WBreak(parent));
	table->elementAt(1, 0)->addWidget(new WText("password :", parent));
	table->elementAt(1, 1)->addWidget(passwordEdit_ = new WLineEdit(parent));
	passwordEdit_->setEchoMode(WLineEdit::Password);
	passwordEdit_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit_));

	//table->elementAt(0, 0)->->addWidget(new WBreak(parent));
	WPushButton* okButton = new WPushButton("Log in");
	homePage->addWidget(okButton);
	//table->elementAt(2, 0)->setColumnSpan(2);
	//table->setHeaderCount(1, Orientation::Vertical);

	okButton->clicked().connect(this, &OutPage::onLogButtonClicked);

	return homePage;
}

Wt::WContainerWidget* OutPage::createRegisterPage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* regPage = new WContainerWidget(parent);
	Wt::WTable* table = new Wt::WTable(regPage);
	table->elementAt(0, 0)->addWidget(new WText("login :", parent));
	table->elementAt(0, 1)->addWidget(loginEdit2_ = new WLineEdit(parent));
	loginEdit2_->setValidator(new WLengthValidator(0, MaxStringSize, loginEdit2_));
	//table->elementAt(1, 0)->addWidget(new WBreak(parent));
	table->elementAt(1, 0)->addWidget(new WText("password :", parent));
	table->elementAt(1, 1)->addWidget(passwordEdit2_ = new WLineEdit(parent));
	passwordEdit2_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit2_));
	passwordEdit2_->setEchoMode(WLineEdit::Password);
	//table->elementAt(1, 0)->addWidget(new WBreak(parent));
	table->elementAt(2, 0)->addWidget(new WText("password2 :", parent));
	table->elementAt(2, 1)->addWidget(passwordEdit3_ = new WLineEdit(parent));
	passwordEdit3_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit3_));
	passwordEdit3_->setEchoMode(WLineEdit::Password);
	//table->elementAt(1, 0)->addWidget(new WBreak(parent));
	WPushButton* regButton = new WPushButton("Register");
	regPage->addWidget(regButton);

	regButton->clicked().connect(this, &OutPage::registerButtonClicked);

	return regPage;
}

OutPage::OutPage(Wt::WContainerWidget* parent, Engine& engine):
	WContainerWidget(parent),
	engine_(engine),
	loginEdit_(nullptr),
	passwordEdit_(nullptr),
	loginEdit2_(nullptr),
	passwordEdit2_(nullptr),
	passwordEdit3_(nullptr)
{
	setStyleClass("homepage");
	setHeight(1024);
	Wt::WStackedWidget* contents = new Wt::WStackedWidget();
	Wt::WMenu* menu = new Wt::WMenu(contents, Wt::Horizontal, this);
	//menu->setInline(true);
	//addWidget(menu);
	//contents->setInline(true);
	addWidget(contents);
	menu->setRenderAsList(false);

	menu->addItem("Home", createHomePage(this));
	menu->addItem("Register", createRegisterPage(this));
}

void OutPage::onLogButtonClicked()
{
	boost::optional<Player> player = engine_.getPlayer(
	                                   loginEdit_->text().toUTF8(), passwordEdit_->text().toUTF8());
	if(player)
		onPlayerLogin(player->id);
	else
	{
		WMessageBox::show("Error", "Wrong login or password.", Ok);
		return;
	}
}

void OutPage::registerButtonClicked()
{
	if(loginEdit2_->text().empty())
		WMessageBox::show("Error", "Empty login", Ok);
	else if(passwordEdit2_->text().empty())
		WMessageBox::show("Error", "Empty password", Ok);
	else if(passwordEdit2_->text() != passwordEdit3_->text())
		WMessageBox::show("Error", "Passwords don't match!", Ok);
	else if(engine_.addPlayer(loginEdit2_->text().toUTF8(), passwordEdit2_->text().toUTF8()))
	{
		WMessageBox::show("Info", "Registration successful", Ok);
		dynamic_cast<WMenu&>(*widget(0)).select(0);
		refresh();
	}
	else
		WMessageBox::show("Error", "Login still exist.", Ok);
}