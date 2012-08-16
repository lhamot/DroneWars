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
	void on_blocklySaveCodeButton_clicked(Wt::WTextArea* hidenLua, Wt::WTextArea* hidenXML);
	void on_blocklyResetCodeButton_clicked(Wt::WContainerWidget* container);
	void refreshCodeMirror(Wt::WContainerWidget*);
	void on_saveCodeButton_clicked(Wt::WTextArea* textarea);
	void on_resetCodeButton_clicked(Wt::WContainerWidget*);

	void createAll();

	std::string name_;
	Engine& engine_;
	Player::ID logged_;
	Wt::WTabWidget* tabWidget_;
};


#endif //__BTA_EDITOR__