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
	boost::filesystem::directory_iterator dir("save/"), end;

	time_t maxtime = 0;
	for(const boost::filesystem::path & p: boost::make_iterator_range(dir, end))
	{
		std::string const fileStr = p.filename().string();
		if(fileStr.find("_save.bta") == 10 && fileStr.size() == 19)
		{
			std::string const strTime = fileStr.substr(0, 10);
			time_t filetime = strtoul(strTime.c_str(), 0, 10);
			maxtime = max(maxtime, filetime);
		}
	}
	if(maxtime)
	{
		std::stringstream ss;
		ss << "save/" << maxtime << "_save.bta";
		load(ss.str());
	}
	else
		construct(univ_);
	start();
}


void Engine::load(std::string const& univName)
{
	UniqueLock lock(univ_.planetsFleetsReportsmutex);
	UniqueLock lockPlayers(univ_.playersMutex);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(std::ios::failure("Can't load from " + univName));
	loadFromStream(loadFile, univ_);
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

	Player::ID const pid = createPlayer(univ_, login, password); //Modifie le joueur ET une planete
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
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.planetsCode.setCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerFleetBlocklyCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setBlocklyCode(code);
	simulation_->reloadPlayer(pid);
}


void Engine::setPlayerPlanetBlocklyCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.playersMutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
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
	boost::copy(univ_.playerMap | boost::adaptors::map_values, back_inserter(playerList));
	return playerList;
}

Player Engine::getPlayer(Player::ID pid) const
{
	SharedLock lock(univ_.playersMutex);
	return mapFind(univ_.playerMap, pid)->second;
}

Planet Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.planetsFleetsReportsmutex);
	return mapFind(univ_.planetMap, coord)->second;
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