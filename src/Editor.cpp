#include "stdafx.h"
#include <Wt/WTextArea>
//#include <Wt/WMessageBox>
#include "Engine.h"
#include "Editor.h"

using namespace Wt;

Editor::Editor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
	Wt::WContainerWidget(parent),
	edit_(nullptr),
	name_(name),
	engine_(engine),
	logged_(pid)
{
	Wt::WTabWidget* tab = new Wt::WTabWidget(this);
	WContainerWidget* blockly = new WContainerWidget(this);
	refreshBlockly(blockly);
	tab->addTab(blockly, "Visual");
	WContainerWidget* codemirror = new WContainerWidget(this);
	refreshCodeMirror(codemirror);
	tab->addTab(codemirror, "Text");
	//refreshBlockly();
	//refreshCodeMirror();

	//addChild(blockly);
	//addChild(codemirror);
}

Editor::~Editor()
{
}

void Editor::refreshBlockly(WContainerWidget* container)
{
	//On recrée son contenue

	//WContainerWidget* container = new WContainerWidget(this);
	container->setId("editorTab");
	container->doJavaScript(
	  "function blocklyLoaded(blockly) {         \n"
	  "  // Called once Blockly is fully loaded. \n"
	  "  window.Blockly" + name_ + " = blockly;               \n"
	  "}                                         \n"
	  "window.blocklyLoaded = blocklyLoaded;"
	);

	WText* frame = new WText(container);
	frame->setTextFormat(Wt::XHTMLUnsafeText);
	frame->setText(
	  "<iframe class=\"blocklyEditor\" src=\"frame.html\"></iframe>"
	);
	//frame->setText("<iframe class=\"blocklyEditor\" src=\"frame.html\"></iframe>");
	frame->setId("blocklyFrame");

	WText* errorMessage = new WText(container);
	new WBreak(container);

	WPushButton* reset = new WPushButton(container);
	reset->setText("Reset");
	if(name_ == "Planet")
	{
		reset->setAttributeValue("onclick",
		                         //"Blockly.clear();\n"
		                         "var xml_text = '<xml xmlns=\"http://www.w3.org/1999/xhtml\"><block type=\"procedures_defreturn\" inline=\"false\" x=\"56\" y=\"52\"><mutation><arg name=\"planet\"></arg><arg name=\"fleets\"></arg></mutation><title name=\"NAME\">AI</title></block></xml>';\n"
		                         "var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);\n"
		                         "Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
		                        );
	}
	else
	{
		reset->setAttributeValue("onclick",
		                         //"Blockly.clear();\n"
		                         "var xml_text = '<xml xmlns=\"http://www.w3.org/1999/xhtml\"><block type=\"procedures_defreturn\" inline=\"false\" x=\"52\" y=\"-232\"><mutation><arg name=\"myFleet\"></arg><arg name=\"otherFleet\"></arg></mutation><title name=\"NAME\">AI:do_gather</title><value name=\"RETURN\"><block type=\"logic_boolean\"><title name=\"BOOL\">TRUE</title></block></value></block><block type=\"procedures_defreturn\" inline=\"false\" x=\"59\" y=\"-129\"><mutation><arg name=\"myFleet\"></arg><arg name=\"otherFleet\"></arg></mutation><title name=\"NAME\">AI:do_fight</title><value name=\"RETURN\"><block type=\"logic_boolean\"><title name=\"BOOL\">TRUE</title></block></value></block><block type=\"procedures_defreturn\" inline=\"false\" x=\"59\" y=\"-16\"><mutation><arg name=\"myFleet\"></arg><arg name=\"planet\"></arg></mutation><title name=\"NAME\">AI:action</title><value name=\"RETURN\"><block type=\"dronewars_fleetaction\" inline=\"true\"><title name=\"ACTION\">Nothing</title></block></value></block></xml>';\n"
		                         "var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);\n"
		                         "Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
		                        );
	}

	WPushButton* load = new WPushButton(container);
	load->setText("Load");
	load->setAttributeValue("onclick",
	                        //"Blockly.clear();\n"
	                        "var xml_text = prompt('Load', '');\n"
	                        "var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);\n"
	                        "Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
	                       );

	WPushButton* saveToXml = new WPushButton(container);
	saveToXml->setText("Save");
	saveToXml->setAttributeValue("onclick",
	                             "var xml = Blockly" + name_ + ".Xml.workspaceToDom(Blockly" + name_ + ".mainWorkspace);\n"
	                             "var xml_text = Blockly" + name_ + ".Xml.domToText(xml);\n"
	                             "alert(xml_text);\n"
	                            );

	WPushButton* saveToLua = new WPushButton(container);
	saveToLua->setText("Valid");
	saveToLua->setAttributeValue("onclick",
	                             "var code = window.Blockly" + name_ + ".Generator.workspaceToCode('lua');\n"
	                             "alert(code);\n"
	                            );

	//reset->clicked().connect(this, &Editor::on_resetBlocklyButton_clicked);
}

void Editor::refreshCodeMirror(WContainerWidget* container)
{
	//On recrée son contenue
	//WContainerWidget* container = new WContainerWidget(this);
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
	reset->clicked().connect(std::bind(&Editor::on_resetCodeButton_clicked, this, container));
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

void Editor::on_resetCodeButton_clicked(WContainerWidget* container)
{
	edit_ = nullptr;

	//On supprime le contenue de codeTab
	while(container->count())
	{
		Wt::WWidget* toDelete = container->widget(0);
		removeWidget(toDelete);
		delete toDelete;
	}

	refreshCodeMirror(container);
}

/*void Editor::on_resetBlocklyButton_clicked()
{
	WMessageBox *box = new WMessageBox("", "", Wt::Icon(), Ok);
	box->
}*/