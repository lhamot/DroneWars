#include "stdafx.h"
#include "OutPage.h"


#include "Engine.h"
#include "TextGetter.h"


using namespace Wt;

Wt::WContainerWidget* OutPage::createHomePage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* homePage = new WContainerWidget(parent);
	homePage->setInline(true);

	Wt::WTable* outTable = new WTable(homePage);

	Wt::WTable* table = new Wt::WTable(homePage);

	table->elementAt(0, 0)->addWidget(new WText(gettext("Login") + " :", parent));
	table->elementAt(0, 1)->addWidget(loginEdit_ = new WLineEdit(parent));
	loginEdit_->setValidator(new WLengthValidator(0, MaxStringSize, loginEdit_));
	table->elementAt(1, 0)->addWidget(new WText(gettext("Password") + " :", parent));
	table->elementAt(1, 1)->addWidget(passwordEdit_ = new WLineEdit(parent));
	passwordEdit_->setEchoMode(WLineEdit::Password);
	passwordEdit_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit_));

	WPushButton* okButton = nullptr;
	table->elementAt(2, 0)->addWidget(
	  okButton = new WPushButton(gettext("Log in"), parent));
	table->elementAt(2, 0)->setColumnSpan(2);
	table->setInline(true);
	outTable->elementAt(0, 0)->addWidget(table);

	okButton->clicked().connect(this, &OutPage::onLogButtonClicked);

	WText* intro = new WText(homePage);
	intro->setStyleClass("manual");
	intro->setWidth(500);
	intro->setTextFormat(Wt::XHTMLUnsafeText);
	intro->setText(gettext("INTRO_TEXT"));
	intro->setInline(true);
	outTable->elementAt(0, 1)->addWidget(intro);

	return homePage;
}


Wt::WContainerWidget* OutPage::createRegisterPage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* regPage = new WContainerWidget(parent);
	Wt::WTable* table = new Wt::WTable(regPage);
	table->elementAt(0, 0)->addWidget(new WText(gettext("Login") + " :", parent));
	table->elementAt(0, 1)->addWidget(loginEdit2_ = new WLineEdit(parent));
	loginEdit2_->setValidator(new WLengthValidator(0, MaxStringSize, loginEdit2_));

	table->elementAt(1, 0)->addWidget(new WText(gettext("Password") + " :", parent));
	table->elementAt(1, 1)->addWidget(passwordEdit2_ = new WLineEdit(parent));
	passwordEdit2_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit2_));
	passwordEdit2_->setEchoMode(WLineEdit::Password);

	table->elementAt(2, 0)->addWidget(new WText(gettext("Password2") + " :", parent));
	table->elementAt(2, 1)->addWidget(passwordEdit3_ = new WLineEdit(parent));
	passwordEdit3_->setValidator(new WLengthValidator(0, MaxStringSize, passwordEdit3_));
	passwordEdit3_->setEchoMode(WLineEdit::Password);

	WPushButton* regButton = new WPushButton(gettext("Register"));
	regPage->addWidget(regButton);

	regButton->clicked().connect(this, &OutPage::registerButtonClicked);

	return regPage;
}

Wt::WContainerWidget* OutPage::createAboutPage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* about = new WContainerWidget(parent);

	WText* intro = new WText(about);
	intro->setStyleClass("manual");
	intro->setWidth(640);
	about->addWidget(intro);
	intro->setTextFormat(Wt::XHTMLUnsafeText);
	intro->setText(gettext("ABOUT_TEXT"));

	return about;
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

	addWidget(contents);
	menu->setRenderAsList(false);

	menu->addItem(gettext("Home"),            createHomePage(this));
	menu->addItem(gettext("Create account"),  createRegisterPage(this));
	menu->addItem(gettext("About DroneWars"), createAboutPage(this));
}


void OutPage::onLogButtonClicked()
{
	boost::optional<Player> player = engine_.getPlayer(
	                                   loginEdit_->text().toUTF8(), passwordEdit_->text().toUTF8());
	if(player)
		onPlayerLogin(player->id);
	else
	{
		WMessageBox::show(gettext("Error"), gettext("Wrong login or password."), Ok);
		return;
	}
}


void OutPage::registerButtonClicked()
{
	if(loginEdit2_->text().empty())
		WMessageBox::show(gettext("Error"), gettext("Empty login"), Ok);
	else if(passwordEdit2_->text().empty())
		WMessageBox::show(gettext("Error"), gettext("Empty password"), Ok);
	else if(passwordEdit2_->text() != passwordEdit3_->text())
		WMessageBox::show(gettext("Error"), gettext("Passwords don't match!"), Ok);
	else if(engine_.addPlayer(loginEdit2_->text().toUTF8(), passwordEdit2_->text().toUTF8()))
	{
		WMessageBox::show(gettext("Info"), gettext("Registration successful"), Ok);
		dynamic_cast<WMenu&>(*widget(0)).select(0);
		refresh();
	}
	else
		WMessageBox::show(gettext("Error"), gettext("Login still exist."), Ok);
}