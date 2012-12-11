#include "stdafx.h"
#include "Engine.h"
#include "Editor.h"
#include "Tools.h"
#include "Rules.h"
#include "TranslationTools.h"
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/thread/locks.hpp>
#include <boost/system/error_code.hpp>
#include <Wt/WTextArea>

using namespace Wt;


static boost::mutex BlocklyFrameMutex_;

class BlocklyEditor : public WContainerWidget
{
	std::string name_;
	Engine& engine_;
	Player::ID const logged_;

public:
	std::function<void()> savedSignal;


	BlocklyEditor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
		WContainerWidget(parent),
		name_(name),
		engine_(engine),
		logged_(pid)
	{
		init();
	}

	~BlocklyEditor()
	{
	}


	void init()
	{
		WContainerWidget* container = new WContainerWidget(this);

		Player const player = engine_.getPlayer(logged_);
		CodeData const& code = (name_ == "Fleet") ?
		                       player.fleetsCode :
		                       player.planetsCode;
		if(name_ != "Fleet" && name_ != "Planet")
			BOOST_THROW_EXCEPTION(std::logic_error("Unexpected name_ value : " + name_));


		container->setId("editorTab" + name_);
		container->doJavaScript(
		  (boost::format(
		     "function blocklyLoaded%1%(blockly) {                                  \n"
		     "  // Called once Blockly is fully loaded.                             \n"
		     "  window.Blockly%1% = blockly;                                        \n"
		     "  var xml_text = '%2%';                                               \n"
		     "  var xml = Blockly%1%.Xml.textToDom(xml_text);                       \n"
		     "  window.Blockly%1%.Xml.domToWorkspace(Blockly%1%.mainWorkspace, xml);\n"
		     "}                                                                     \n"
		     "window.blocklyLoaded%1% = blocklyLoaded%1%;                           \n"
		   ) % name_ % escape(code.getBlocklyCode())).str()
		);

		size_t const plLvl = player.getTutoLevel(CoddingLevelTag);
		boost::filesystem::path const filename =
		  boost::str(boost::format("%1%Frame%2%.html") % name_ % 0);
		{
			boost::unique_lock<boost::mutex> lock(BlocklyFrameMutex_);
			if(boost::filesystem::exists(filename) == false)
			{
				boost::filesystem::ofstream file(filename, std::ios::out | std::ios::binary);
				if(file.is_open() == false)
					BOOST_THROW_EXCEPTION(std::ios::failure(
					                        "Can't open file to write : " + filename.string()));
				getBlocklyHTML(plLvl, name_, file);
			}
		}

		WContainerWidget* tutosContainer = new WContainerWidget(container);
		tutosContainer->addStyleClass("manual");
		WText* totuText = new WText(tutosContainer);
		totuText->setTextFormat(Wt::XHTMLUnsafeText);
		totuText->setText(getTutoText(plLvl));

		WText* frame = new WText(container);
		frame->setTextFormat(Wt::XHTMLUnsafeText);
		frame->setText(
		  "<iframe class=\"blocklyEditor\" src=\"" + filename.string() + "\"></iframe>"
		);
		frame->setId("blocklyFrame" + name_);

		new WText(container); //errorMessage
		new WBreak(container);

		WPushButton* reload = new WPushButton(container);
		reload->setText(gettext("Reload"));
		reload->clicked().connect(
		  boost::bind(&BlocklyEditor::refresh, this));

		if(plLvl > 6)
		{
			WPushButton* load = new WPushButton(container);
			load->setText(gettext("Import"));
			load->setAttributeValue(
			  "onclick",
			  (boost::format(
			     "var xml_text = prompt('Load', '');\n"
			     "var xml = Blockly%1%.Xml.textToDom(xml_text);\n"
			     "Blockly%1%.Xml.domToWorkspace(Blockly%1%.mainWorkspace, xml);\n"
			   ) % name_).str()
			);

			WPushButton* saveToXml = new WPushButton(container);
			saveToXml->setText(gettext("Export"));
			saveToXml->setAttributeValue(
			  "onclick",
			  (boost::format(
			     "var xml = Blockly%1%.Xml.workspaceToDom(Blockly%1%.mainWorkspace);\n"
			     "var xml_text = Blockly%1%.Xml.domToText(xml);\n"
			     "alert(xml_text);\n"
			   ) % name_).str()
			);
		}

		WPushButton* saveToLua = new WPushButton(container);
		saveToLua->setText(gettext("Valid"));
		saveToLua->setAttributeValue(
		  "onClick",
		  (boost::format(
		     "var code = window.Blockly%1%.Generator.workspaceToCode('lua');\n"
		     "document.getElementById(\"hidenLua%1%\").innerHTML = code;\n"
		     "var xml = Blockly%1%.Xml.workspaceToDom(Blockly%1%.mainWorkspace);\n"
		     "var xml_text = Blockly%1%.Xml.domToText(xml);\n"
		     "document.getElementById(\"hidenXML%1%\").innerHTML = xml_text;\n"
		     "document.getElementById(\"saveToLua2%1%\").click();\n"
		   ) % name_).str()
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
		  boost::bind(&BlocklyEditor::on_blocklySaveCodeButton_clicked, this, hidenLua, hidenXML));
	}

	void refresh()
	{
		//std::cout << __FUNCTION__ << std::endl;
		WWidget* container = widget(0);
		Player const player = engine_.getPlayer(logged_);
		CodeData const& code = (name_ == "Fleet") ?
		                       player.fleetsCode :
		                       player.planetsCode;
		if(name_ != "Fleet" && name_ != "Planet")
			BOOST_THROW_EXCEPTION(std::logic_error("Unexpected name_ value : " + name_));
		container->doJavaScript(
		  (boost::format(
		     "function blocklyLoaded%1%(blockly) {                               \n"
		     "  // Called once Blockly is fully loaded.                          \n"
		     "  window.Blockly%1% = blockly;                                     \n"
		     "  var xml_text = '%2%';                                            \n"
		     "  var xml = window.Blockly%1%.Xml.textToDom(xml_text);             \n"
		     "  window.Blockly%1%.Xml.domToWorkspace(Blockly%1%.mainWorkspace, xml);\n"
		     "}                                                                  \n"
		     "window.blocklyLoaded%1% = blocklyLoaded%1%;                        \n"
		     "window.Blockly%1%.mainWorkspace.clear();                           \n"
		     "var xml_text = '%2%';                                              \n"
		     "var xml = window.Blockly%1%.Xml.textToDom(xml_text);               \n"
		     "window.Blockly%1%.Xml.domToWorkspace(Blockly%1%.mainWorkspace, xml);\n"
		   ) % name_ % escape(code.getBlocklyCode())).str()
		);
	}


	void on_blocklySaveCodeButton_clicked(WTextArea* hidenLua, WTextArea* hidenXML)
	{
		std::string code = hidenLua->text().toUTF8();
		std::string xmlCode = hidenXML->text().toUTF8();
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
		Wt::WMessageBox::show(gettext("Info"), gettext("Code successfully saved"), Wt::Ok);

		if(savedSignal)
			savedSignal();
	}
};


class TextEditor : public WContainerWidget
{
	std::string name_;
	Engine& engine_;
	Player::ID const logged_;

public:
	TextEditor(Wt::WContainerWidget* parent, std::string const& name, Engine& engine, Player::ID pid):
		WContainerWidget(parent),
		name_(name),
		engine_(engine),
		logged_(pid)
	{
		TextEditor::refresh();
	}

	~TextEditor()
	{
		std::cout << __FUNCTION__ << std::endl;
	}


	void refresh()
	{
		//On supprime le contenue de codeTab
		while(count())
		{
			Wt::WWidget* toDelete = widget(0);
			removeWidget(toDelete);
			delete toDelete;
		}

		WTextArea* edit = new WTextArea(this);
		edit->setRows(80);
		edit->setColumns(120);

		WText* errorMessage = new WText(this);
		new WBreak(this);

		WPushButton* reset = new WPushButton(this);
		reset->setText(gettext("Reload"));

		WPushButton* save = new WPushButton(this);
		save->setText(gettext("Save"));
		save->setId("saveCodeButton" + name_);

		WPushButton* help = new WPushButton(this);
		help->setText(gettext("Help"));
		help->setLink(WLink(getenv("LANG") + std::string("/API.htm")));
		help->setLinkTarget(Wt::TargetNewWindow);

		edit->setId(name_ + "TextArea");
		edit->setValidator(new WLengthValidator(0, Player::MaxCodeSize, edit));
		edit->doJavaScript(
		  (boost::format(
		     "var editor%1% = CodeMirror.fromTextArea(document.getElementById(\"%1%TextArea\"), {"
		     "  tabMode: \"indent\","
		     "  matchBrackets: true,"
		     "  lineNumbers: true,"
		     "  theme: \"cobalt\","
		     "  onHighlightComplete: function(editor) {editor%1%.save();}"
		     "});\n"
		     "window.editor%1% = editor%1%;\n"
		   ) % name_).str()
		);

		save->clicked().connect(std::bind(&TextEditor::on_saveCodeButton_clicked, this, edit));
		reset->clicked().connect(std::bind(&TextEditor::refresh, this));
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


	void on_saveCodeButton_clicked(WTextArea* textarea)
	{
		std::string code = textarea->text().toUTF8();
		if(name_ == "Fleet")
			engine_.setPlayerFleetCode(logged_, code);
		else if(name_ == "Planet")
			engine_.setPlayerPlanetCode(logged_, code);
		else
			BOOST_THROW_EXCEPTION(std::logic_error("Bad code editor type"));
		Wt::WMessageBox::show(gettext("Info"), gettext("Code successfully saved"), Wt::Ok);
	}
};



Editor::Editor(Wt::WContainerWidget* parent,
               std::string const& name,
               Engine& engine,
               Player::ID pid,
               int tabIndex):
	Wt::WContainerWidget(parent),
	name_(name),
	engine_(engine),
	logged_(pid),
	tabWidget_(nullptr)
{
	tabWidget_ = new Wt::WTabWidget(this);

	BlocklyEditor* bled = nullptr;
	TextEditor* ted = nullptr;
	tabWidget_->addTab(new WContainerWidget(this), "dummy");
	tabWidget_->addTab(bled = new BlocklyEditor(this, name_, engine_, logged_), gettext("Visual"));
	tabWidget_->addTab(ted = new TextEditor(this, name_, engine_, logged_), gettext("Text"));
	tabWidget_->setCurrentIndex(tabIndex + 1);

	bled->savedSignal = [ = ]() {ted->refresh();};

	WWidget* dummy = tabWidget_->widget(0);
	tabWidget_->removeTab(dummy);
	delete dummy;
	dummy = nullptr;
}


Editor::~Editor()
{
}

int Editor::currentIndex() const
{
	return tabWidget_->currentIndex();
}
