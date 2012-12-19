#ifndef __DRONEWARS_SIMULATION__
#define __DRONEWARS_SIMULATION__

#include <boost/thread/locks.hpp>
#include <luabind/luabind.hpp>
#include "Model.h"

namespace LuaTools
{
class LuaEngine;
}

struct PlayerCodes
{
	luabind::object fleetsCode;
	luabind::object planetsCode;
};
typedef std::unordered_map<Player::ID, PlayerCodes> PlayerCodeMap;
typedef std::unordered_multimap<Coord, Fleet> FleetCoordMap;


class Simulation
{
	Simulation(Simulation const&);
	Simulation& operator = (Simulation const&);
public:
	Simulation(Universe& univ);

	void reloadPlayer(Player::ID pid);

	void loop();

private:
	void round(LuaTools::LuaEngine&,
	           PlayerCodeMap& codesMap,
	           std::vector<Signal>& signals);
	//luabind::object registerCode(
	//  LuaTools::LuaEngine& luaEngine,
	//  Player::ID const pid, CodeData& code, time_t time, bool isFleet);
	void updatePlayersCode(LuaTools::LuaEngine& luaEngine,
	                       PlayerCodeMap& codesMap,
	                       std::vector<Signal>& signals);
	void save(std::string const& univName) const;
	void removeOldSaves() const;

	mutable boost::shared_mutex reloadPlayerMutex_;
	std::set<Player::ID> playerToReload_;
	Universe& univ_;
};


#endif //__DRONEWARS_SIMULATION__