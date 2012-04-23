#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "Model.h"
#include <boost/thread.hpp>

#pragma warning(push)
#pragma warning(disable:4512 4127 4244 4121 4100)
#include <boost/python.hpp>
#pragma warning(pop)

class Engine
{
public:
	Engine();

	~Engine();

	void start();

	void stop();

	void construct();

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

	typedef boost::shared_mutex Mutex;

private:
	typedef std::multimap<Coord, Fleet, CompCoord> FleetCoordMap;


	void round();
	void loop();
	void execPlanet(boost::python::object, Planet& planet, time_t time);
	bool execFleet(boost::python::object, Fleet& fleet, FleetCoordMap& fleetMap, time_t time);
	boost::python::object registerCode(
	  Player::ID const pid, std::string const& module, std::string const& code, time_t time);

	struct PyCodes
	{
		boost::python::object fleetsCode;
		boost::python::object planetsCode;
	};

	Universe univ_;
	std::map<Player::ID, PyCodes> codesMap_;
	boost::thread simulating_;
	mutable Mutex mutex_;
};

#endif //__BTA_ENGINE__