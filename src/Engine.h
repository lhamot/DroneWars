#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4121 4100)
//#include <boost/python.hpp>
#include <luabind/luabind.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#pragma warning(pop)

#include "Model.h"
#include <unordered_map>

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

	bool addPlayer(std::string const& login, std::string const& password);

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

	boost::optional<Player> getPlayer(std::string const& login, std::string const& password) const;

	void load(std::string const& univName);

private:

	class Simulation
	{
		Simulation(Simulation const&);
		Simulation& operator = (Simulation const&);
	public:
		Simulation(Universe& univ);

		void save(std::string const& univName) const;

		void loop();

		void start();

		void stop();

		void reloadPlayer(Player::ID pid)
		{
			boost::unique_lock<Universe::Mutex> lock(mutex_);
			playerToReload_.insert(pid);
		}

	private:
		typedef std::unordered_multimap<Coord, Fleet> FleetCoordMap;
		typedef std::pair<FleetCoordMap::const_iterator, FleetCoordMap::const_iterator> FleetRange;
		struct PlayerCodes
		{
			luabind::object fleetsCode;
			luabind::object planetsCode;
		};
		typedef std::unordered_map<Player::ID, PlayerCodes> PlayerCodeMap;

		void round(LuaTools::LuaEngine&, PlayerCodeMap& codesMap);
		void execPlanet(LuaTools::LuaEngine& luaEngine, luabind::object, Planet& planet, time_t time, std::vector<Fleet const*> const& fleetList);
		bool execFleet(LuaTools::LuaEngine& luaEngine, luabind::object, Fleet& fleet, FleetCoordMap& fleetMap, time_t time);
		luabind::object registerCode(
		  LuaTools::LuaEngine& luaEngine,
		  Player::ID const pid, std::string const& code, time_t time);

		boost::shared_mutex mutex_;
		std::set<Player::ID> playerToReload_;
		Universe& univ_;
	};

	Universe univ_;
	Simulation simulation_;
	boost::thread simulating_;
};

#endif //__BTA_ENGINE__