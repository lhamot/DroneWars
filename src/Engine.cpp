#include "Engine.h"

#include <vector>
#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>

#include "PythonUniverse.h"
#include "PyTools.h"

typedef boost::unique_lock<Engine::Mutex> UniqueLock;
typedef boost::shared_lock<Engine::Mutex> SharedLock;

template<typename M>
typename M::const_iterator
mapFind(M const& map, typename M::key_type key)
{
	typename M::const_iterator iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}

template<typename M>
typename M::iterator
mapFind(M& map, typename M::key_type key)
{
	typename M::iterator iter = map.find(key);
	if(iter == map.end())
		BOOST_THROW_EXCEPTION(std::logic_error("Can't find item"));
	return iter;
}


Engine::Engine()
{
	start();
}


Engine::~Engine()
{
	stop();
}


void Engine::start()
{
	simulating_ = boost::thread(&Engine::loop, this);
}


void Engine::stop()
{
	simulating_.interrupt();
	simulating_.join();
}


void Engine::construct()
{
	UniqueLock lock(mutex_);
	::construct(univ_);
}


void Engine::load(std::string const& univName)
{
	UniqueLock lock(mutex_);
	using namespace std;
	ifstream loadFile(univName, ios::in | ios::binary);
	if(loadFile.is_open() == false)
		BOOST_THROW_EXCEPTION(std::ios::failure("Can't load from " + univName));
	loadFromStream(loadFile, univ_);
}


void Engine::save(std::string const& saveName) const
{
	SharedLock lock(mutex_);

	using namespace std;
	std::string const newSaveName = saveName + ".new";
	{
		ofstream saveFile(newSaveName.c_str(), ios::out | ios::binary);
		if(saveFile.is_open() == false)
			BOOST_THROW_EXCEPTION(std::ios::failure("Can't save in " + newSaveName));
		saveToStream(univ_, saveFile);
	}
	std::string const ansSaveName = saveName + ".ans";
	remove(ansSaveName.c_str());
	struct stat buf;
	if(stat(saveName.c_str(), &buf ) == 0)
		rename(saveName.c_str(), ansSaveName.c_str());
	rename(newSaveName.c_str(), saveName.c_str());
	std::cout << "OK" << std::endl;
}


void Engine::addPlayer(Player const&) //player
{
	UniqueLock lock(mutex_);
	//TODO
}


std::vector<Fleet> Engine::getPlayerFleets(Player::ID pid) const
{
	SharedLock lock(mutex_);
	std::vector<Fleet> fleetList;
	BOOST_FOREACH(Fleet const & fleet, univ_.fleetList)
	{
		if(fleet.playerId == pid)
			fleetList.push_back(fleet);
	}
	return fleetList;
}


std::vector<Planet> Engine::getPlayerPlanets(Player::ID pid) const
{
	SharedLock lock(mutex_);
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
	UniqueLock lock(mutex_);
	mapFind(univ_.playerMap, pid)->second.fleetsCode = code;
}


void Engine::setPlayerPlanetCode(Player::ID pid, std::string const& code)
{
	UniqueLock lock(mutex_);
	mapFind(univ_.playerMap, pid)->second.planetsCode = code;
}


std::string Engine::getPlayerFleetCode(Player::ID pid) const
{
	SharedLock lock(mutex_);
	return 	mapFind(univ_.playerMap, pid)->second.fleetsCode;
}


std::string Engine::getPlayerPlanetCode(Player::ID pid) const
{
	SharedLock lock(mutex_);
	return 	mapFind(univ_.playerMap, pid)->second.planetsCode;
}


std::vector<Player> Engine::getPlayers() const
{
	SharedLock lock(mutex_);
	std::vector<Player> playerList;
	BOOST_FOREACH(Universe::PlayerMap::value_type const & nvp, univ_.playerMap)
	{
		playerList.push_back(nvp.second);
	}
	return playerList;
}

Player Engine::getPlayer(Player::ID pid) const
{
	SharedLock lock(mutex_);
	return mapFind(univ_.playerMap, pid)->second;
}

Planet Engine::getPlanet(Coord coord) const
{
	SharedLock lock(mutex_);
	return mapFind(univ_.planetMap, coord)->second;
}


boost::python::object Engine::registerCode(
  Player::ID const pid, std::string const& module, std::string const& code)
try
{
	using namespace boost::python;
	using namespace std;

	static size_t counter = 0;
	std::string implModule = module + '_' + boost::lexical_cast<std::string>(counter);

	object main_module = import("__main__");
	object main_namespace = main_module.attr("__dict__");
	//object local_module = main_namespace.attr(implModule.c_str());
	//object local_namespace = local_module.attr("__dict__");
	object fileObject = exec(code.c_str(), main_namespace);
	object funcAI = eval("AI", main_namespace);
	assert(funcAI.ptr());

	++counter;

	return funcAI;

	//std::string const moduleName = module + boost::lexical_cast<std::string>(pid);
	//return boost::python::object();
}
catch(boost::python::error_already_set const&)
{
	PyErr_Print();
	mapFind(univ_.playerMap, pid)->second.eventList.push_back(
	  Event(time(0), Event::FleetCodeError, PyTools::getPyStdErr()));
	return boost::python::object();
}

bool canBuild(Planet const& planet, Building::Type type)
{
	auto iter = std::find_if(planet.taskQueue.begin(), planet.taskQueue.end(), 
		[&](Task const& task){return task.value == static_cast<size_t>(type);});
	if(iter == planet.taskQueue.end())
		return true;
	else
		return false;
}

void addTask(Planet& planet, Building::Type building)
{
	auto const buIter = planet.buildingMap.find(building);
	size_t const buLevel = (buIter == planet.buildingMap.end())? 0: buIter->second;
	size_t const duration = static_cast<size_t>(pow(buLevel + 1., 1.5) * 10);
	planet.taskQueue.push_back(Task(Task::UpgradeBuilding, building, time(0), duration));
}

bool canStop(Planet& planet, Building::Type type)
{
	return true;
}

void stopTask(Planet& planet, Task::Type tasktype, Building::Type building)
{
	auto iter = std::find_if(planet.taskQueue.begin(), planet.taskQueue.end(), [&]
	(Task const& task)
	{
		return task.type == tasktype && task.value == static_cast<size_t>(building);
	});

	if(iter != planet.taskQueue.end())
		planet.taskQueue.erase(iter);
}

void Engine::execPlanet(boost::python::object code, Planet& planet)
try
{
	PlanetActionList list;
	code(planet, boost::ref(list));
	BOOST_FOREACH(PlanetAction const& action, list)
	{
		switch(action.action)
		{
		case PlanetAction::Building:
			{
				if(canBuild(planet, action.building))
					addTask(planet, action.building);
			}
			break;
		case PlanetAction::StopBuilding:
			{
				if(canStop(planet, action.building))
					stopTask(planet, Task::UpgradeBuilding, action.building);
			}
			break;
		};
	}
}
catch(boost::python::error_already_set const&)
{
	PyErr_Print();
	mapFind(univ_.playerMap, planet.playerId)->second.eventList.push_back(
	  Event(time(0), Event::FleetCodeError, PyTools::getPyStdErr()));
}


void Engine::execFleet(boost::python::object code, Fleet& fleet)
try
{
	//code(fleet);
}
catch(boost::python::error_already_set const&)
{
	PyErr_Print();
	mapFind(univ_.playerMap, fleet.playerId)->second.eventList.push_back(
	  Event(time(0), Event::FleetCodeError, PyTools::getPyStdErr()));
}

static size_t const RoundSecond = 5;


void Engine::round()
{
	std::cout << "Mise a jour";
	UniqueLock lock(mutex_);

	univ_.time += RoundSecond;
	
	PyTools::PythonEngine pyEngine;
	initDroneWars();
	std::string const boringWarnings = PyTools::getPyStdErr();

	//Rechargement de tout les code flote/planet de tout les joueur(chargement dans python)
	//std::map<Player::ID, PyCodes> codesMap;
	BOOST_FOREACH(Universe::PlayerMap::value_type & playerNVP, univ_.playerMap)
	{
		Player const& player = playerNVP.second;
		PyCodes newCodes =
		{
			boost::python::object(),//registerCode(player.id, "Fleet", player.fleetsCode),
			registerCode(player.id, "Planet", player.planetsCode)
		};
		codesMap_[player.id] = newCodes;
	}

	//Les planètes
	BOOST_FOREACH(Universe::PlanetMap::value_type & planetNVP, univ_.planetMap)
	{
		Planet& planet = planetNVP.second;
		if(planet.playerId != Player::NoId)
			execPlanet(codesMap_[planet.playerId].planetsCode, planet);

		//On met de coté les taches qui sont périmée
		auto splitIter = std::remove_if(planet.taskQueue.begin(), planet.taskQueue.end(),[&]
		(Task const& task)
		{
			return (task.lauchTime + task.duration) <= univ_.time;
		});

		//On les excecutes
		for(auto iter = splitIter; iter != planet.taskQueue.end(); ++iter)
		{
			Task const& task = *iter;
			switch(task.type)
			{
			case Task::UpgradeBuilding:
				planet.buildingMap[static_cast<Building::Type>(task.value)] += 1;
				break;
			}
		}

		//Puis ont les supprime
		planet.taskQueue.erase(splitIter, planet.taskQueue.end());
	}

	//Les flotes
	BOOST_FOREACH(Fleet & fleet, univ_.fleetList)
		execFleet(codesMap_[fleet.playerId].fleetsCode, fleet);

	codesMap_.clear();

	std::cout << "  OK" << std::endl;
}


void Engine::loop()
{
	//PyTools::PythonEngine pyEngine;
	//initDroneWars();

	time_t newUpdate;
	time(&newUpdate);
	newUpdate += RoundSecond;

	while(false == boost::this_thread::interruption_requested())
	{
		time_t now;
		time(&now);

		if(newUpdate <= now)
		{
			//std::cout << newUpdate << " " << now << std::endl;
			round();
			newUpdate += RoundSecond;
		}
		else
			boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}

