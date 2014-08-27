//! @file
//! @author Loïc HAMOT

#include "stdafx.h"
#include "Engine.h"

#include "Tools.h"
#include "Simulation.h"
#include "UnivManip.h"

using namespace std;
using namespace boost;

typedef boost::unique_lock<boost::shared_mutex> UniqueLock; //!< Verou en écriture
typedef boost::shared_lock<boost::shared_mutex> SharedLock; //!< Verou en lecture

Engine::Engine(DataBase::ConnectionInfo const& connInfo,
               size_t minRoundDuration) :
	simulation_(new Simulation(univ_, connInfo, minRoundDuration)),
	connInfo_(connInfo)
{
	filesystem::directory_iterator dir("save/"), end;

	time_t maxtime = 0;
	size_t version = 1;
	//! Recherche la dernière sauvegarde
	for(const filesystem::path& p : make_iterator_range(dir, end))
	{
		string const fileStr = p.filename().string();
		if((fileStr.find("_save.bta") == 10) &&
		   (fileStr.size() == 19 || fileStr.size() == 20))
		{
			string const strTime = fileStr.substr(0, 10);
			time_t filetime = strtoul(strTime.c_str(), 0, 10);
			maxtime = max(maxtime, filetime);
			if(fileStr.size() == 20)
				version = 2;
		}
	}
	if(maxtime)
	{
		//! Si elle est trouvée on la charge
		stringstream ss;
		ss << "save/" << maxtime << "_save.bta";
		if(version == 2)
			ss << "2";
		load(ss.str(), version);
		DataBase database(connInfo);
		std::vector<Player> players = database.getPlayers();
		for(Player const& player : players)
		{
			if(player.mainPlanet == UndefinedCoord)
				simulation_->createMainPlanet(player.id);
		}
		start();
	}
}


void Engine::load(string const& univName, size_t version)
{
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(ios::failure("Can't load from " + univName));
	else if(version == 2)
		loadFromStream_v2(loadFile, univ_);
	else
		BOOST_THROW_EXCEPTION(logic_error("Unexpected archive file version"));
}


Engine::~Engine()
{
	stop();
}


void Engine::start()
{
	simulating_ = boost::thread(&Simulation::loop, boost::ref(*simulation_));
}


void Engine::stop()
{
	simulating_.interrupt();
	simulating_.join();
}


void Engine::addPlayer(Player::ID pid)
{
	simulation_->createMainPlanet(pid); //Modifie une planete
}


vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	vector<Fleet> fleetList;
	fleetList.reserve(1000);
	for(Fleet const& fleet : univ_.fleetMap | adaptors::map_values)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(Fleet(fleet, DontCopyMemory));
	}
	return fleetList;
}


vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	vector<Planet> planetList;
	planetList.reserve(1000);
	for(Planet const& planet : univ_.planetMap | adaptors::map_values)
	{
		if(planet.playerId == pid)
			planetList.push_back(Planet(planet, DontCopyMemory));
	}
	return planetList;
}


optional<Planet> Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.mutex);
	auto iter = univ_.planetMap.find(coord);
	return iter == univ_.planetMap.end() ?
	       optional<Planet>() :
	       iter->second;
}

vector<Planet> Engine::getPlanets(vector<Coord> const& coordVect) const
{
	SharedLock lock(univ_.mutex);
	vector<Planet> result;
	result.reserve(coordVect.size());
	for(Coord const& coord : coordVect)
	{
		auto iter = univ_.planetMap.find(coord);
		if(iter != univ_.planetMap.end())
			result.push_back(iter->second);
	}
	return result;
}

boost::optional<Fleet> Engine::getFleet(Fleet::ID fid) const
{
	SharedLock lock(univ_.mutex);
	auto iter = univ_.fleetMap.find(fid);
	if(iter == univ_.fleetMap.end())
		return none;
	else
		return iter->second;
}


TimeInfo Engine::getTimeInfo() const
{
	SharedLock lock(univ_.mutex);
	TimeInfo info = {univ_.roundDuration, simulation_->getUnivTime()};
	return info;
}


void Engine::reloadPlayer(Player::ID pid)
{
	simulation_->reloadPlayer(pid);
}

void Engine::createUniverse(bool keepPlayers)
{
	stop();
	DataBase database(connInfo_);
	auto players = database.getPlayers();
	univ_ = Universe();
	database.clear(keepPlayers);
	construct(univ_, database);
	start();
}

