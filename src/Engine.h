#ifndef __BTA_ENGINE__
#define __BTA_ENGINE__

#include "stdafx.h"
#include "Model.h"
#include "DataBase.h"

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

	void load(std::string const& univName, size_t version);

	//Requetes qui ne modifient pas la base
	std::vector<Fleet> getPlayerFleets(Player::ID pid) const;

	std::vector<Planet> getPlayerPlanets(Player::ID pid) const;

	boost::optional<Planet> getPlanet(Coord coord) const;

	std::vector<Planet> getPlanets(std::vector<Coord> const& coord) const;

	Fleet getFleet(Fleet::ID fid) const;

	//Requetes qui modifient la base
	bool addPlayer(DataBase& database, std::string const& login, std::string const& password);

	TimeInfo getTimeInfo() const;

	void reloadPlayer(Player::ID pid);

private:
	Universe univ_;
	std::unique_ptr<Simulation> simulation_;
	boost::thread simulating_;
};

#endif //__BTA_ENGINE__