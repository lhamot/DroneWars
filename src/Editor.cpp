#include "stdafx.h"
#include <Wt/WTextArea>
#include "Editor.h"

using namespace Wt;

Editor::Editor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
	Wt::WContainerWidget(parent),
	edit_(nullptr),
	name_(name),
	engine_(engine),
	logged_(pid)
{
	//WContainerWidget *codeTab = new WContainerWidget(this);
	refresh();
}

Editor::~Editor()
{
}

void Editor::refresh()
{
	edit_ = nullptr;

	//On supprime le contenue de codeTab
	while(count())
	{
		Wt::WWidget* toDelete = widget(0);
		removeWidget(toDelete);
		delete toDelete;
	}

	//On recrée son contenue
	WContainerWidget* container = new WContainerWidget(this);
	edit_ = new WTextArea(container);
	edit_->setRows(80);
	edit_->setColumns(120);

	WText* errorMessage = new WText(container);
	new WBreak(container);

	WPushButton* reset = new WPushButton(container);
	reset->setText("Reset");

	WPushButton* save = new WPushButton(container);
	save->setText("Save");

	edit_->setId(name_ + "TextArea");
	edit_->setValidator(new WLengthValidator(0, Player::MaxCodeSize, edit_));
	edit_->doJavaScript(
	  "var editor = CodeMirror.fromTextArea(document.getElementById(\"" + name_ + "TextArea\"), {"
	  "tabMode: \"indent\","
	  "matchBrackets: true,"
	  "lineNumbers: true,"
	  "theme: \"cobalt\","
	  "onHighlightComplete: function(editor) {editor.save();}"
	  "});"
	);

	save->clicked().connect(this, &Editor::on_saveCodeButton_clicked);
	reset->clicked().connect(this, &Editor::on_resetCodeButton_clicked);
	if(name_ == "Fleet")
	{
		CodeData code = engine_.getPlayerFleetCode(logged_);
		edit_->setText(code.getCode().c_str());
		errorMessage->setText(code.getLastError().c_str());
	}
	else if(name_ == "Planet")
	{
		CodeData code = engine_.getPlayerPlanetCode(logged_);
		edit_->setText(code.getCode().c_str());
		errorMessage->setText(code.getLastError().c_str());
	}
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));
}

void Editor::on_saveCodeButton_clicked()
{
	std::string code = edit_->text().toUTF8();
	if(name_ == "Fleet")
		engine_.setPlayerFleetCode(logged_, code);
	else if(name_ == "Planet")
		engine_.setPlayerPlanetCode(logged_, code);
}

void Editor::on_resetCodeButton_clicked()
{
	refresh();
}