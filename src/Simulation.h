#ifndef __DRONEWARS_SIMULATION__
#define __DRONEWARS_SIMULATION__

#include <boost/thread/locks.hpp>
#include <luabind/luabind.hpp>
#include "Model.h"
#include <boost/chrono.hpp>

namespace LuaTools
{
class LuaEngine;
}

struct PlayerCodes
{
	//Si une clef n'est pas trouvé dans une ObjectMap, ca veut dire que le code
	// à été invalidé
	typedef std::map<std::string, luabind::object> ObjectMap;
	ObjectMap fleetsCode;
	ObjectMap planetsCode;
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

	double getUnivTime();

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
	boost::chrono::system_clock::time_point roundStart;
	Universe& univ_;
	mutable boost::thread savingThread_;
};


#endif //__DRONEWARS_SIMULATION__