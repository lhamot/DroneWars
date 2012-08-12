#ifndef __BTA_EDITOR__
#define __BTA_EDITOR__

#include "stdafx.h"

#include "Player.h"

class Engine;

class Editor : public Wt::WContainerWidget
{
public:
	Editor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid);

	~Editor();

private:
	void refreshBlockly(Wt::WContainerWidget*);
	void refreshCodeMirror(Wt::WContainerWidget*);
	void on_saveCodeButton_clicked();
	void on_resetCodeButton_clicked(WContainerWidget*);

	Wt::WTextArea* edit_;
	std::string name_;
	Engine& engine_;
	Player::ID logged_;
};


#endif //__BTA_EDITOR__