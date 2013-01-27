#include "stdafx.h"
#include "Rules.h"
#include <boost/format.hpp>



void getBlocklyHTML(size_t tutoLevel, std::string const& codecontext, std::ostream& out)
{
	enum Categ
	{
	  All,
	  Fleet,
	  Planet
	};

	auto filter = [&](char const * str, size_t planetLevel, size_t fleetLevel)
	{
		return codecontext == "Fleet" ?
       fleetLevel <= tutoLevel ? str : "" :
		       planetLevel <= tutoLevel ? str : "";
	};

	size_t const Never = size_t(-1);

	out <<
	    "<html>\n"
	    "  <head>\n"
	    "    <meta charset=\"utf-8\">\n"
	    "    <script type=\"text/javascript\" src=\"blockly/blockly_compressed.js\"></script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/language/fr/_messages.js\">    </script>\n" <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/control.js\">       </script>\n", 1, 1) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/text.js\">          </script>\n", 10, 10) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/lists.js\">         </script>\n", 10, 10) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/logic.js\">         </script>\n", 1, 0) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/math.js\">          </script>\n", 1, 1) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/procedures.js\">    </script>\n", 0, 0) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly/language/common/variables.js\">     </script>\n", 0, 0) <<
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua.js\">           </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/control.js\">   </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/text.js\">      </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/lists.js\">     </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/logic.js\">     </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/math.js\">      </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/procedures.js\"></script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/generators/lua/variables.js\"> </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/language/fr/dronewars.js\">    </script>\n"
	    "    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars.js\"></script>\n" <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Fleet.js\">       </script>\n", 3, 3) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Planet.js\">      </script>\n", 1, 1) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_RessourceSet.js\"></script>\n", 3, 3) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_Coord.js\">       </script>\n", Never, 0) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_FleetAction.js\"> </script>\n", Never, 0) <<
	    filter("    <script type=\"text/javascript\" src=\"blockly_adds/language/common/dronewars_PlanetAction.js\"></script>\n", 0, Never) <<
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
	    ) % codecontext;
}

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;
typedef boost::upgrade_lock<Universe::Mutex> UpgradeLock;
typedef boost::upgrade_to_unique_lock<Universe::Mutex> UpToUniqueLock;

void checkTutos(Universe& univ_, std::vector<Signal>& signals)
{
	UpgradeLock lockPlayer(univ_.playersMutex);
	SharedLock lockAllOthers(univ_.planetsFleetsReportsmutex);

	std::vector<Player*> wisePlayer;
	wisePlayer.reserve(univ_.playerMap.size());
	for(Player & player: univ_.playerMap | boost::adaptors::map_values)
	{
		size_t const plLvl = player.getTutoLevel(CoddingLevelTag);
		switch(plLvl)
		{
		case 0: //! Cas 0 : Créer une mine de métal
		{
			Planet const& planet = mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] > 0)
				wisePlayer.push_back(&player);
			break;
		}
		case 1: //! Cas 1 : Créer fabrique SI mine de métal a 4
		{
			Planet const& planet = mapFind(univ_.planetMap, player.mainPlanet)->second;
			if(planet.buildingList[Building::MetalMine] >= 4 &&
			   planet.buildingList[Building::Factory] > 0)
				wisePlayer.push_back(&player);
			break;
		}
		case 2: //! Cas 2: Créer Vaisseau
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::ShipMade)
				{
					wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 3: //! Cas 3: Créer 3 flottes
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::ShipMade)
				{
					size_t count = boost::range::count_if(
					                 univ_.fleetMap | boost::adaptors::map_values,
					                 bind(&Fleet::playerId, _1) == player.id);
					if(count == 3)
						wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 4: //! Cas 4: Créer 3 flottes composé de 5 mosquito exactement
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::ShipMade)
				{
					size_t count = boost::range::count_if(
					                 univ_.fleetMap | boost::adaptors::map_values,
					                 [&](Fleet const & fleet)
					{
						return fleet.playerId == player.id &&
						       fleet.shipList[Ship::Mosquito] == 5;
					});
					if(count >= 3)
						wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 5: //! Cas 5: Envoyez 6 flottes dans 6 endroit différent
		{
			std::set<Coord, CompCoord> fleetCoords;
			for(Fleet const & fleet: univ_.fleetMap | boost::adaptors::map_values)
			{
				if(fleet.playerId == player.id)
				{
					fleetCoords.insert(fleet.coord);
					if(fleetCoords.size() >= 6)
					{
						wisePlayer.push_back(&player);
						break;
					}
				}
			}
			break;
		}
		case 6: //! Cas 6: Récoltez ressources
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::PlanetHarvested)
				{
					wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 7: //! Cas 7: Rapporter ressources
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::FleetDrop)
				{
					wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 8: //! Cas 8: Colonisation
		{
			for(Signal const & sig: signals)
			{
				if(sig.playerID == player.id && sig.event.type == Event::PlanetColonized)
				{
					wisePlayer.push_back(&player);
					break;
				}
			}
			break;
		}
		case 9: break;
		case 10: break;
		case 11: break;
		case 12: break;
		case 13: break;
		}
	}

	UpToUniqueLock writeLock(lockPlayer);
	for(Player * player: wisePlayer)
		player->tutoDisplayed[CoddingLevelTag] += 1;
}


bool fleetCanSeePlanet(Fleet const& fleet, Planet const& planet)
{
	return (fleet.playerId == planet.playerId) || (planet.coord.X > 0);
}

void updateScore(Universe& univ)
{
	std::map<Player::ID, size_t> playerScore;

	for(Planet const & planet: univ.planetMap | boost::adaptors::map_values)
	{
		size_t score = 0;
		for(size_t type = 0; type < Building::Count; ++type)
			score += Building::List[type].price.tab[0] * planet.buildingList[type];
		for(size_t type = 0; type < Cannon::Count; ++type)
			score += Cannon::List[type].price.tab[0] * planet.cannonTab[type];

		playerScore[planet.playerId] += score;
	}

	for(Fleet const & fleet: univ.fleetMap | boost::adaptors::map_values)
	{
		size_t score = 0;
		for(size_t type = 0; type < Ship::Count; ++type)
			score += Ship::List[type].price.tab[0] * fleet.shipList[type];

		playerScore[fleet.playerId] += score;
	}

	for(Player & player: univ.playerMap | boost::adaptors::map_values)
		player.score = playerScore[player.id];
}