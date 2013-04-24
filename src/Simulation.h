#ifndef __DRONEWARS_SIMULATION__
#define __DRONEWARS_SIMULATION__

#include <boost/thread/locks.hpp>
#include <luabind/luabind.hpp>
#include "Model.h"
#include "DataBase.h"
#include <boost/chrono.hpp>
#include "PlayerCodes.h"

namespace LuaTools
{
class LuaEngine;
}


class Simulation
{
	Simulation(Simulation const&);
	Simulation& operator = (Simulation const&);
public:
	Simulation(Universe& univ);

	void reloadPlayer(Player::ID pid);

	void loop();

	double getUnivTime();

private:
	void round(LuaTools::LuaEngine&,
	           PlayerCodeMap& codesMap,
	           std::vector<Event>& events);
	void updatePlayersCode(LuaTools::LuaEngine& luaEngine,
	                       PlayerCodeMap& codesMap,
	                       std::vector<Event>& events);
	void save(std::string const& univName) const;
	void removeOldSaves() const;

	mutable boost::shared_mutex reloadPlayerMutex_;
	std::set<Player::ID> playerToReload_;
	boost::chrono::system_clock::time_point roundStart;
	Universe& univ_;
	DataBase database_;
	mutable boost::thread savingThread_;
};


#endif //__DRONEWARS_SIMULATION__