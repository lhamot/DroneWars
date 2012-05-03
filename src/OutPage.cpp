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
#pragma warning(pop)


using namespace Wt;

Wt::WContainerWidget* OutPage::createHomePage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* homePage = new WContainerWidget(parent);
	homePage->addWidget(new WText("login :", parent));
	homePage->addWidget(loginEdit_ = new WLineEdit(parent));
	homePage->addWidget(new WBreak(parent));
	homePage->addWidget(new WText("password :", parent));
	homePage->addWidget(passwordEdit_ = new WLineEdit(parent));
	passwordEdit_->setEchoMode(WLineEdit::Password);
	homePage->addWidget(new WBreak(parent));
	WPushButton* okButton = new WPushButton("Log in");
	homePage->addWidget(okButton);

	okButton->clicked().connect(this, &OutPage::onLogButtonClicked);

	return homePage;
}

Wt::WContainerWidget* OutPage::createRegisterPage(Wt::WContainerWidget* parent)
{
	Wt::WContainerWidget* regPage = new WContainerWidget(parent);
	regPage->addWidget(new WText("login :", parent));
	regPage->addWidget(loginEdit2_ = new WLineEdit(parent));
	regPage->addWidget(new WBreak(parent));
	regPage->addWidget(new WText("password :", parent));
	regPage->addWidget(passwordEdit2_ = new WLineEdit(parent));
	passwordEdit2_->setEchoMode(WLineEdit::Password);
	regPage->addWidget(new WBreak(parent));
	regPage->addWidget(new WText("password2 :", parent));
	regPage->addWidget(passwordEdit3_ = new WLineEdit(parent));
	passwordEdit3_->setEchoMode(WLineEdit::Password);
	regPage->addWidget(new WBreak(parent));
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
	Wt::WStackedWidget *contents = new Wt::WStackedWidget();
	Wt::WMenu *menu = new Wt::WMenu(contents, Wt::Horizontal, this);
	//menu->setInline(true);
	//addWidget(menu);
	//contents->setInline(true);
	addWidget(contents);
  //menu->setRenderAsList(true);

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
		 WMessageBox::show("Info", "Registration successful", Ok);
	else
		 WMessageBox::show("Error", "Login still exist.", Ok);
}