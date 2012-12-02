#include "stdafx.h"
#include "Engine.h"
#include "Editor.h"
#include "Tools.h"
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/thread/locks.hpp>
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
		std::cout << __FUNCTION__ << std::endl;
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
		  "function blocklyLoaded" + name_ + "(blockly) {                         \n"
		  "  // Called once Blockly is fully loaded.                              \n"
		  "  window.Blockly" + name_ + " = blockly;                               \n"
		  "  var xml_text = '" + escape(code.getBlocklyCode()) + "';              \n"
		  "  var xml = Blockly" + name_ + ".Xml.textToDom(xml_text);              \n"
		  "  window.Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
		  "}                                                                      \n"
		  "window.blocklyLoaded" + name_ + " = blocklyLoaded" + name_ + ";        \n"
		);

		size_t const plLvl = player.getTutoLevel(CoddingLevelTag);
		boost::format const filename = boost::format("%1%Frame%2%.html") % name_ % 0;
		{
			auto filter = [&](char const * str, size_t needed)
			{
				return needed <= plLvl ? str : "";
			};

			auto forName = [&](char const * str, char const * name)
			{
				return name == name_ ? str : "";
			};

			std::string const filenameStr = filename.str();
			boost::unique_lock<boost::mutex> lock(BlocklyFrameMutex_);
			if(boost::filesystem::exists(filenameStr) == false)
			{
				std::ofstream file(filenameStr.c_str());
				file <<
				     "<html>\n"
				     "  <head>\n"
				     "    <meta charset=\"utf-8\">\n"
				     "    <script type=\"text/javascript\" src=\"blockly/demos/blockly_compressed.js\"></script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/fr/_messages.js\">    </script>\n" <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/control.js\">       </script>\n", 0) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/text.js\">          </script>\n", 10) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/lists.js\">         </script>\n", 10) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/logic.js\">         </script>\n", 0) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/math.js\">          </script>\n", 0) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/procedures.js\">    </script>\n", 0) <<
				     filter("    <script type=\"text/javascript\" src=\"blockly/language/common/variables.js\">     </script>\n", 0) <<
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua.js\">           </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/control.js\">   </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/text.js\">      </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/lists.js\">     </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/logic.js\">     </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/math.js\">      </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/procedures.js\"></script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/variables.js\"> </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/fr/dronewars.js\">    </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars.js\"></script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Fleet.js\">       </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Planet.js\">      </script>\n"
				     "    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_RessourceSet.js\"></script>\n" <<
				     forName("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Coord.js\">       </script>\n", "Fleet") <<
				     forName("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_FleetAction.js\"> </script>\n", "Fleet") <<
				     forName("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_PlanetAction.js\"></script>\n", "Planet") <<
				     "    <style>\n"
				     "      html, body {\n"
				     "        background-color: #fff;\n"
				     "        margin: 0;\n"
				     "        padding:0;\n"
				     "        overflow: hidden;\n"
				     "      }\n"
				     "      .blocklySvg {\n"
				     "        height: 100%;\n"
				     "        width: 100%;\n"
				     "      }\n"
				     "    </style>\n" <<
				     boost::format(
				       "    <script>\n"
				       "			  function load() {\n"
				       "				  	if (window.parent.blocklyLoaded%1%)\n"
				       "					  		window.parent.blocklyLoaded%1%(Blockly);\n"
				       "					  else\n"
				       "						  	setTimeout(function () { load();}, 100);\n"
				       "			  }\n"
				       "        function init() {\n"
				       "            Blockly.inject(document.body, { path: 'blockly/' });\n"
				       "            load();"
				       "        }\n"
				       "    </script>\n"
				       "  </head>\n"
				       "  <body onload=\"init()\">\n"
				       "  </body>\n"
				       "</html>"
				     ) % name_;
			}
		}

		WText* totuText = new WText(container);
		totuText->addStyleClass("manual");
		totuText->setTextFormat(Wt::XHTMLUnsafeText);
		switch(plLvl)
		{
		case 0: totuText->setText(gettext("BLOCKLY_TUTO_0")); break;
		case 1: totuText->setText(gettext("BLOCKLY_TUTO_1")); break;
		case 2: totuText->setText(gettext("BLOCKLY_TUTO_2")); break;
		case 3: totuText->setText(gettext("BLOCKLY_TUTO_3")); break;
		case 4: totuText->setText(gettext("BLOCKLY_TUTO_4")); break;
		case 5: totuText->setText(gettext("BLOCKLY_TUTO_5")); break;
		}

		WText* frame = new WText(container);
		frame->setTextFormat(Wt::XHTMLUnsafeText);
		frame->setText(
		  "<iframe class=\"blocklyEditor\" src=\"" + filename.str() + "\"></iframe>"
		);
		frame->setId("blocklyFrame" + name_);

		new WText(container); //errorMessage
		new WBreak(container);

		WPushButton* reload = new WPushButton(container);
		reload->setText(gettext("Reload"));
		reload->clicked().connect(
		  boost::bind(&BlocklyEditor::refresh, this));

		WPushButton* load = new WPushButton(container);
		load->setText(gettext("Import"));
		load->setAttributeValue("onclick",
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
		  boost::bind(&BlocklyEditor::on_blocklySaveCodeButton_clicked, this, hidenLua, hidenXML));
	}

	void refresh()
	{
		std::cout << __FUNCTION__ << std::endl;
		WWidget* container = widget(0);
		Player const player = engine_.getPlayer(logged_);
		CodeData const& code = (name_ == "Fleet") ?
		                       player.fleetsCode :
		                       player.planetsCode;
		if(name_ != "Fleet" && name_ != "Planet")
			BOOST_THROW_EXCEPTION(std::logic_error("Unexpected name_ value : " + name_));
		container->doJavaScript(
		  "function blocklyLoaded" + name_ + "(blockly) {                         \n"
		  "  // Called once Blockly is fully loaded.                              \n"
		  "  window.Blockly" + name_ + " = blockly;                               \n"
		  "  var xml_text = '" + escape(code.getBlocklyCode()) + "';                      \n"
		  "  var xml = window.Blockly" + name_ + ".Xml.textToDom(xml_text);              \n"
		  "  window.Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
		  "}                                                                      \n"
		  "window.blocklyLoaded" + name_ + " = blocklyLoaded" + name_ + ";        \n"
		  "window.Blockly" + name_ + ".mainWorkspace.clear();\n"
		  "var xml_text = '" + escape(code.getBlocklyCode()) + "';                      \n"
		  "var xml = window.Blockly" + name_ + ".Xml.textToDom(xml_text);              \n"
		  "window.Blockly" + name_ + ".Xml.domToWorkspace(Blockly" + name_ + ".mainWorkspace, xml);\n"
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
		  "var editor" + name_ + " = CodeMirror.fromTextArea(document.getElementById(\"" + name_ + "TextArea\"), {"
		  "tabMode: \"indent\","
		  "matchBrackets: true,"
		  "lineNumbers: true,"
		  "theme: \"cobalt\","
		  "onHighlightComplete: function(editor) {editor" + name_ + ".save();}"
		  "});\n"
		  "window.editor" + name_ + " = editor" + name_ + ";\n"
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
	std::cout << "Construction Editor " << name_ << " avec index " << tabIndex << std::endl;
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
