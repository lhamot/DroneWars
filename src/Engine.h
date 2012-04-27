#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "Model.h"
#include <boost/thread.hpp>

#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4121 4100)
//#include <boost/python.hpp>
#include <luabind/luabind.hpp>
#pragma warning(pop)

namespace LuaTools
{
class LuaEngine;
}

class Engine
{
public:
	Engine();

	~Engine();

	void start();

	void stop();

	void load(std::string const& univName);

	void save(std::string const& univName) const;

	void addPlayer(Player const& player);

	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	void setPlayerFleetCode(Player::ID pid, std::string const& code);

	void setPlayerPlanetCode(Player::ID pid, std::string const& code);

	std::string getPlayerFleetCode(Player::ID pid) const;

	std::string getPlayerPlanetCode(Player::ID pid) const;

	std::vector<Player> getPlayers() const;

	Player getPlayer(Player::ID pid) const;

	Planet getPlanet(Coord coord) const;

	Fleet getFleet(Fleet::ID fid);

private:

	class Simulation
	{
		Simulation(Simulation const&);
		Simulation& operator = (Simulation const&);
	public:
		Simulation(Universe &univ):univ_(univ)
		{
		}

		void loop();
		
		void start();

		void stop();

		void reloadPlayer(Player::ID pid)
		{
			playerToReload_.insert(pid);
		}

	private:
		typedef std::multimap<Coord, Fleet, CompCoord> FleetCoordMap;
		struct PlayerCodes
		{
			luabind::object fleetsCode;
			luabind::object planetsCode;
		};
		typedef std::map<Player::ID, PlayerCodes> PlayerCodeMap;

		void round(LuaTools::LuaEngine&, PlayerCodeMap& codesMap);
		void execPlanet(luabind::object, Planet& planet, time_t time);
		bool execFleet(luabind::object, Fleet& fleet, FleetCoordMap& fleetMap, time_t time);
		luabind::object registerCode(
			LuaTools::LuaEngine& luaEngine,
			Player::ID const pid, std::string const& code, time_t time);

		std::set<Player::ID> playerToReload_;
		Universe &univ_;
	};

	Universe univ_;
	Simulation simulation_;
	boost::thread simulating_;
};

#endif //__BTA_ENGINE__