#include "Engine.h"

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/filesystem.hpp>

#include "Tools.h"

using namespace std;
using namespace boost;

typedef boost::unique_lock<Universe::Mutex> UniqueLock;
typedef boost::shared_lock<Universe::Mutex> SharedLock;

Engine::Engine():
	simulation_(univ_)
{
	boost::filesystem::directory_iterator dir("."), end;

	time_t maxtime = 0;
	BOOST_FOREACH(const boost::filesystem::path & p, std::make_pair(dir, end))
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
		ss << maxtime << "_save.bta";
		load(ss.str());
	}
	else
		construct(univ_);
	start();
}


void Engine::load(std::string const& univName)
{
	UniqueLock lock(univ_.mutex);
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
	simulating_ = boost::thread(&Simulation::loop, boost::ref(simulation_));
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
	UniqueLock lock(univ_.mutex);

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

	Player::ID const pid = createPlayer(univ_, login, password);
	simulation_.reloadPlayer(pid);
	return true;
}


std::vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	std::vector<Fleet> fleetList;
	BOOST_FOREACH(Fleet const & fleet, univ_.fleetMap | boost::adaptors::map_values)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(fleet);
	}
	return fleetList;
}


std::vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	std::vector<Planet> planetList;
	BOOST_FOREACH(Universe::PlanetMap::value_type const & planetNVP, univ_.planetMap)
	{
		if(planetNVP.second.playerId == pid)
			planetList.push_back(planetNVP.second);
	}
	return planetList;
}


void Engine::setPlayerFleetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.mutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.fleetsCode.setCode(code);
	simulation_.reloadPlayer(pid);
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(univ_.mutex);
	if(code.size() > Player::MaxCodeSize)
		BOOST_THROW_EXCEPTION(InvalidData("code"));
	mapFind(univ_.playerMap, pid)->second.planetsCode.setCode(code);
	simulation_.reloadPlayer(pid);
}


CodeData Engine::getPlayerFleetCode(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return 	mapFind(univ_.playerMap, pid)->second.fleetsCode;
}


CodeData Engine::getPlayerPlanetCode(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return 	mapFind(univ_.playerMap, pid)->second.planetsCode;
}


std::vector<Player> Engine::getPlayers() const
{
	SharedLock lock(univ_.mutex);
	std::vector<Player> playerList;
	BOOST_FOREACH(Universe::PlayerMap::value_type const & nvp, univ_.playerMap)
	{
		playerList.push_back(nvp.second);
	}
	return playerList;
}

Player Engine::getPlayer(Player::ID pid) const
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.playerMap, pid)->second;
}

Planet Engine::getPlanet(Coord coord) const
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.planetMap, coord)->second;
}

Fleet Engine::getFleet(Fleet::ID fid)
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.fleetMap, fid)->second;
}


boost::optional<Player> Engine::getPlayer(
  std::string const& login, std::string const& password) const
{
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

FightReport Engine::getFightReport(size_t id)
{
	SharedLock lock(univ_.mutex);
	return mapFind(univ_.reportMap, id)->second;
}

