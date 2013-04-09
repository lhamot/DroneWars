#include "stdafx.h"
#include "Engine.h"

#include "Tools.h"
#include "Simulation.h"

using namespace std;
using namespace boost;

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;

Engine::Engine():
	simulation_(new Simulation(univ_))
{
	filesystem::directory_iterator dir("save/"), end;

	time_t maxtime = 0;
	size_t version = 1;
	for(const filesystem::path & p: make_iterator_range(dir, end))
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
		stringstream ss;
		ss << "save/" << maxtime << "_save.bta";
		if(version == 2)
			ss << "2";
		load(ss.str(), version);
	}
	else
	{
		DataBase database;
		construct(univ_, database);
	}
	start();
}


void Engine::load(string const& univName, size_t version)
{
	UniqueLock lock(univ_.planetsFleetsReportsmutex);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(ios::failure("Can't load from " + univName));
	if(version == 1)
		loadFromStream_v1(loadFile, univ_);
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


Coord Engine::addPlayer(Player::ID pid)
{
	UniqueLock lock(univ_.planetsFleetsReportsmutex);
	return createPlayer(univ_, pid); //Modifie une planete
}


vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	vector<Fleet> fleetList;
	for(Fleet const & fleet: univ_.fleetMap | adaptors::map_values)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(fleet);
	}
	return fleetList;
}


vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	vector<Planet> planetList;
	for(Universe::PlanetMap::value_type const & planetNVP: univ_.planetMap)
	{
		if(planetNVP.second.playerId == pid)
			planetList.push_back(planetNVP.second);
	}
	return planetList;
}


optional<Planet> Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	auto iter = univ_.planetMap.find(coord);
	return iter == univ_.planetMap.end() ?
	       optional<Planet>() :
	       iter->second;
}

vector<Planet> Engine::getPlanets(vector<Coord> const& coordVect) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	vector<Planet> result;
	result.reserve(coordVect.size());
	for(Coord const & coord: coordVect)
	{
		auto iter = univ_.planetMap.find(coord);
		if(iter != univ_.planetMap.end())
			result.push_back(iter->second);
	}
	return result;
}

Fleet Engine::getFleet(Fleet::ID fid) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	return mapFind(univ_.fleetMap, fid)->second;
}


TimeInfo Engine::getTimeInfo() const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	TimeInfo info = {univ_.roundDuration, simulation_->getUnivTime()};
	return info;
}


void Engine::reloadPlayer(Player::ID pid)
{
	simulation_->reloadPlayer(pid);
}