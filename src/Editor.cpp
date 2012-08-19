#include "stdafx.h"
#include <Wt/WTextArea>
#include "Engine.h"
#include "Editor.h"
#include "TextGetter.h"

using namespace Wt;

Editor::Editor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
	Wt::WContainerWidget(parent),
	name_(name),
	engine_(engine),
	logged_(pid),
	tabWidget_(nullptr)
{
	tabWidget_ = new Wt::WTabWidget(this);
	WContainerWidget* blockly = new WContainerWidget(this);
	refreshBlockly(blockly);
	tabWidget_->addTab(blockly, gettext("Visual"));
	WContainerWidget* codeMirrorTab = new WContainerWidget(this);
	refreshCodeMirror(codeMirrorTab);
	tabWidget_->addTab(codeMirrorTab, gettext("Text"));
}


Editor::~Editor()
{
}

void Editor::refreshBlockly(WContainerWidget* mainContainer)
{
	WContainerWidget* container = new WContainerWidget(mainContainer);

	CodeData code;
	if(name_ == "Fleet")
		code = engine_.getPlayerFleetCode(logged_);
	else if(name_ == "Planet")
		code = engine_.getPlayerPlanetCode(logged_);
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Unexpected name_ value : " + name_));

	container->setId("editorTab" + name_);
	container->doJavaScript(
	  "function blocklyLoaded" + name_ + "(blockly) {                         \n"
	  "  // Called once Blockly is fully loaded.                              \n"
	  "  window.Blockly" + name_ + " = blockly;                               \n"
	  "  var xml_text = '" + code.getBlocklyCode() + "';                      \n"
	  "  var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);              \n"
	  "  window.Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
	  "}                                                                      \n"
	  "window.blocklyLoaded" + name_ + " = blocklyLoaded" + name_ + ";        \n"
	);

	WText* frame = new WText(container);
	frame->setTextFormat(Wt::XHTMLUnsafeText);
	frame->setText(
	  "<iframe class=\"blocklyEditor\" src=\"" + name_ + "Frame.html\"></iframe>"
	);
	//frame->setText("<iframe class=\"blocklyEditor\" src=\"frame.html\"></iframe>");
	frame->setId("blocklyFrame" + name_);

	WText* errorMessage = new WText(container);
	new WBreak(container);

	WPushButton* reload = new WPushButton(container);
	reload->setText(gettext("Reload"));
	reload->clicked().connect(
	  boost::bind(&Editor::on_blocklyResetCodeButton_clicked, this, mainContainer));

	WPushButton* load = new WPushButton(container);
	load->setText(gettext("Import"));
	load->setAttributeValue("onclick",
	                        //"Blockly.clear();\n"
	                        "var xml_text = prompt('Load', '');\n"
	                        "var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);\n"
	                        "Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
	                       );

	WPushButton* saveToXml = new WPushButton(container);
	saveToXml->setText(gettext("Export"));
	saveToXml->setAttributeValue("onclick",
	                             "var xml = Blockly" + name_ + ".Xml.workspaceToDom(Blockly" + name_ + ".mainWorkspace);\n"
	                             "var xml_text = Blockly" + name_ + ".Xml.domToText(xml);\n"
	                             "alert(xml_text);\n"
	                            );

	WPushButton* saveToLua = new WPushButton(container);
	saveToLua->setText(gettext("Valid"));
	saveToLua->setAttributeValue(
	  "onClick",
	  "var code = window.Blockly" + name_ + ".Generator.workspaceToCode('lua');\n"
	  "document.getElementById(\"hidenLua" + name_ + "\").innerHTML = code;\n"
	  "var xml = Blockly" + name_ + ".Xml.workspaceToDom(Blockly" + name_ + ".mainWorkspace);\n"
	  "var xml_text = Blockly" + name_ + ".Xml.domToText(xml);\n"
	  "document.getElementById(\"hidenXML" + name_ + "\").innerHTML = xml_text;\n"
	  "document.getElementById(\"saveToLua2" + name_ + "\").click();\n"
	);
	WPushButton* saveToLua2 = new WPushButton(container);
	saveToLua2->setObjectName("saveToLua2");
	saveToLua2->setId("saveToLua2" + name_);
	saveToLua2->hide();

	WTextArea* hidenLua = new WTextArea(container);
	hidenLua->setId("hidenLua" + name_);
	hidenLua->setAttributeValue("style", "display:none;");

	WTextArea* hidenXML = new WTextArea(container);
	hidenXML->setId("hidenXML" + name_);
	hidenXML->setAttributeValue("style", "display:none;");

	saveToLua2->clicked().connect(
	  boost::bind(&Editor::on_blocklySaveCodeButton_clicked, this, hidenLua, hidenXML));
}

void Editor::on_blocklySaveCodeButton_clicked(WTextArea* hidenLua, WTextArea* hidenXML)
{
	std::string code = hidenLua->text().toUTF8();
	std::string xmlCode = hidenXML->text().toUTF8();
	std::cout << code << std::endl;
	if(name_ == "Fleet")
	{
		engine_.setPlayerFleetBlocklyCode(logged_, xmlCode);
		engine_.setPlayerFleetCode(logged_, code);
	}
	else if(name_ == "Planet")
	{
		engine_.setPlayerPlanetBlocklyCode(logged_, xmlCode);
		engine_.setPlayerPlanetCode(logged_, code);
	}
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));

	tabWidget_->removeTab(tabWidget_->widget(1));
	WContainerWidget* codeMirrorTab = new WContainerWidget(this);
	refreshCodeMirror(codeMirrorTab);
	tabWidget_->addTab(codeMirrorTab, gettext("Text"));
}

void Editor::on_blocklyResetCodeButton_clicked(WContainerWidget* container)
{
	//On supprime le contenue de codeTab
	while(container->count())
	{
		Wt::WWidget* toDelete = container->widget(0);
		container->removeWidget(toDelete);
		delete toDelete;
	}

	refreshBlockly(container);
}


void Editor::refreshCodeMirror(WContainerWidget* container)
{
	WTextArea* edit = new WTextArea(container);
	edit->setRows(80);
	edit->setColumns(120);

	WText* errorMessage = new WText(container);
	new WBreak(container);

	WPushButton* reset = new WPushButton(container);
	reset->setText(gettext("Reload"));

	WPushButton* save = new WPushButton(container);
	save->setText(gettext("Save"));
	save->setId("saveCodeButton" + name_);

	edit->setId(name_ + "TextArea");
	edit->setValidator(new WLengthValidator(0, Player::MaxCodeSize, edit));
	edit->doJavaScript(
	  "var editor" + name_ + " = CodeMirror.fromTextArea(document.getElementById(\"" + name_ + "TextArea\"), {"
	  "tabMode: \"indent\","
	  "matchBrackets: true,"
	  "lineNumbers: true,"
	  "theme: \"cobalt\","
	  "onHighlightComplete: function(editor) {editor" + name_ + ".save();}"
	  "});\n"
	  "window.editor" + name_ + " = editor" + name_ + ";\n"
	);

	save->clicked().connect(std::bind(&Editor::on_saveCodeButton_clicked, this, edit));
	reset->clicked().connect(std::bind(&Editor::on_resetCodeButton_clicked, this, container));
	if(name_ == "Fleet")
	{
		CodeData code = engine_.getPlayerFleetCode(logged_);
		edit->setText(code.getCode().c_str());
		errorMessage->setText(code.getLastError().c_str());
	}
	else if(name_ == "Planet")
	{
		CodeData code = engine_.getPlayerPlanetCode(logged_);
		edit->setText(code.getCode().c_str());
		errorMessage->setText(code.getLastError().c_str());
	}
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));
}

void Editor::on_saveCodeButton_clicked(WTextArea* textarea)
{
	std::string code = textarea->text().toUTF8();
	std::cout << code << std::endl;
	if(name_ == "Fleet")
		engine_.setPlayerFleetCode(logged_, code);
	else if(name_ == "Planet")
		engine_.setPlayerPlanetCode(logged_, code);
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));
}

void Editor::on_resetCodeButton_clicked(WContainerWidget* container)
{
	//On supprime le contenue de codeTab
	while(container->count())
	{
		Wt::WWidget* toDelete = container->widget(0);
		container->removeWidget(toDelete);
		delete toDelete;
	}

	refreshCodeMirror(container);
}

