#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "stdafx.h"
#include "Model.h"

class Simulation;

struct TimeInfo
{
	double roundDuration;
	double univTime;
};


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

	void load(std::string const& univName);

	//Requetes qui ne modifient pas la base
	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	CodeData getPlayerFleetCode(Player::ID pid) const;

	CodeData getPlayerPlanetCode(Player::ID pid) const;

	std::vector<Player> getPlayers() const;

	Player getPlayer(Player::ID pid) const;

	boost::optional<Planet> getPlanet(Coord coord) const;

	std::vector<Planet> getPlanets(std::vector<Coord> const& coord) const;

	Fleet getFleet(Fleet::ID fid) const;

	boost::optional<Player> getPlayer(std::string const& login, std::string const& password) const;

	FightReport getFightReport(size_t id) const;

	//Requetes qui modifient la base
	bool addPlayer(std::string const& login, std::string const& password);

	void setPlayerFleetCode(Player::ID pid, std::string const& code);

	void setPlayerPlanetCode(Player::ID pid, std::string const& code);

	void setPlayerFleetBlocklyCode(Player::ID pid, std::string const& code);

	void setPlayerPlanetBlocklyCode(Player::ID pid, std::string const& code);

	void incrementTutoDisplayed(Player::ID pid, std::string const& tutoName);

	TimeInfo getTimeInfo() const;

private:
	Universe univ_;
	std::unique_ptr<Simulation> simulation_;
	boost::thread simulating_;
};

#endif //__BTA_ENGINE__