#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include <unordered_map>

#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4121 4100)
//#include <boost/python.hpp>
#include <luabind/luabind.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#pragma warning(pop)

#include "Model.h"
#include "Simulation.h"


typedef std::pair<FleetCoordMap::const_iterator, FleetCoordMap::const_iterator> FleetRange;


class Engine
{
public:
	struct InvalidData : public std::runtime_error
	{
		InvalidData(std::string const& str): std::runtime_error(str) {}
	};

	Engine();

	~Engine();

	void start();

	void stop();

	bool addPlayer(std::string const& login, std::string const& password);

	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	void setPlayerFleetCode(Player::ID pid, std::string const& code);

	void setPlayerPlanetCode(Player::ID pid, std::string const& code);

	CodeData getPlayerFleetCode(Player::ID pid) const;

	CodeData getPlayerPlanetCode(Player::ID pid) const;

	std::vector<Player> getPlayers() const;

	Player getPlayer(Player::ID pid) const;

	Planet getPlanet(Coord coord) const;

	Fleet getFleet(Fleet::ID fid);

	boost::optional<Player> getPlayer(std::string const& login, std::string const& password) const;

	FightReport getFightReport(size_t id);

	void load(std::string const& univName);

private:
	Universe univ_;
	Simulation simulation_;
	boost::thread simulating_;
};

#endif //__BTA_ENGINE__