#include "OutPage.h"

#pragma warning(push)
#pragma warning(disable:4251 4275 4505)
#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WDefaultLayout>
#pragma warning(pop)


using namespace Wt;

OutPage::OutPage(Wt::WContainerWidget* parent, Engine& engine):
	WContainerWidget(parent),
	engine_(engine),
	loginEdit_(nullptr),
	passwordEdit_(nullptr)
{
	/*WDefaultLayout *layout = new WDefaultLayout(this);
	setLayout(layout);
	layout->addWidget(new WText("login", this));
	layout->addWidget(new WLineEdit(this));
	WPushButton* okButton = new WPushButton("Login");
	layout->addWidget(okButton);*/
	WWidget* toto = nullptr;
	addWidget(toto = new WText("login", this));
	toto->setInline(false);
	addWidget(loginEdit_ = new WLineEdit(this));
	loginEdit_->setInline(false);
	WPushButton* okButton = new WPushButton("Login");
	addWidget(okButton);
	okButton->setInline(false);

	okButton->clicked().connect(this, &OutPage::onLogButtonClicked);
}

void OutPage::onLogButtonClicked()
{
	boost::optional<Player> player = engine_.getPlayer(loginEdit_->text().toUTF8(), "");
	if(player)
		onPlayerLogin(player->id);
}