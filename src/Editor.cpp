#include "stdafx.h"
#include "Engine.h"
#include "Editor.h"
#include "Tools.h"
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <Wt/WTextArea>

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

	tabWidget_->currentChanged().connect(this, &Editor::on_tab_changed);
}


void Editor::on_tab_changed(int index)
{
	/*switch(index)
	{
	case 0:
	{
		std::cout << name_ << std::endl;
		char const* const TutoTag = (name_ == "Fleet")?
			"BlocklyFleetView":
			"BlocklyPlanetView";
		char const* const text = (name_ == "Fleet")?
			gettext("BLOCKLY_FLEET_TUTOS"):
			gettext("BLOCKLY_PLANET_TUTOS");
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(TutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), text, Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, TutoTag);
		}
	}
	break;
	case 1:
	{
		char const* const TutoTag = "CodeMirrorView";
		Player const player = engine_.getPlayer(logged_);
		if(player.tutoDisplayed.find(TutoTag) == player.tutoDisplayed.end())
		{
			Wt::WMessageBox::show(gettext("Tutorial"), gettext("CODE_MIRROR_TUTOS"), Wt::Ok);
			engine_.incrementTutoDisplayed(logged_, TutoTag);
		}
	}
	break;
	}*/
}


Editor::~Editor()
{
}

void Editor::refreshBlockly(WContainerWidget* mainContainer)
{
	WContainerWidget* container = new WContainerWidget(mainContainer);

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
	  "  var xml_text = '" + escape(code.getBlocklyCode()) + "';                      \n"
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

		std::ofstream file(filename.str().c_str());
		file <<
		     "<html>\n"
		     "  <head>\n"
		     "    <meta charset=\"utf-8\">\n"
		     "    <script type=\"text/javascript\" src=\"blockly/demos/blockly_compressed.js\"></script>\n"
		     "    <script type=\"text/javascript\" src=\"blockly/generators/JavaScript.js\">         </script>\n"
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
		       "        function init() {\n"
		       "            Blockly.inject(document.body, { path: 'blockly/' });\n"
		       "            window.parent.blocklyLoaded%1%(Blockly);\n"
		       "        }\n"
		       "    </script>\n"
		       "  </head>\n"
		       "  <body onload=\"init()\">\n"
		       "  </body>\n"
		       "</html>"
		     ) % name_;
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

	WPushButton* help = new WPushButton(container);
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

