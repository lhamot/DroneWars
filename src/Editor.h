#ifndef __BTA_EDITOR__
#define __BTA_EDITOR__

#include <string>
#include <Wt/WContainerWidget>
#include "Engine.h"

class Editor : public Wt::WContainerWidget
{
public:
	Editor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid);

	~Editor();

private:
	void refresh();
	void on_saveCodeButton_clicked();
	void on_resetCodeButton_clicked();

	Wt::WTextArea* edit_;
	std::string name_;
	Engine& engine_;
	Player::ID logged_;
};


#endif //__BTA_EDITOR__