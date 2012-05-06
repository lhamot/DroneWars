#ifndef __BTA_OUT_PAGE__
#define __BTA_OUT_PAGE__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4512 4275 4505)
#include <Wt/WContainerWidget>
#pragma warning(pop)

#include "Engine.h"

namespace Wt
{
class WLineEdit;
}

class OutPage : public Wt::WContainerWidget
{
public:
	OutPage(Wt::WContainerWidget* parent, Engine& engine);

	std::function<void(Player::ID)> onPlayerLogin;

private:
	void onLogButtonClicked();
	void registerButtonClicked();
	Wt::WContainerWidget* createHomePage(Wt::WContainerWidget* parent);
	Wt::WContainerWidget* createRegisterPage(Wt::WContainerWidget* parent);

	Engine& engine_;
	Wt::WLineEdit* loginEdit_;
	Wt::WLineEdit* passwordEdit_;
	Wt::WLineEdit* loginEdit2_;
	Wt::WLineEdit* passwordEdit2_;
	Wt::WLineEdit* passwordEdit3_;
};

#endif //__BTA_OUT_PAGE__