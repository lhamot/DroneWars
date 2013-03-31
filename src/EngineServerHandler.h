#undef printf //A cause de libintl qui a une macro printf, et thrift qui definie une methode printf
#pragma warning(push)
#pragma warning(disable: 4512 4100 4099 4244 4127 4267 4706)
#include "gen-cpp/EngineServer.h"
#include "gen-cpp/thrift_types.h"
#pragma warning(pop)
#include "Engine.h"



class EngineServerHandler : virtual public ndw::EngineServerIf
{
	Engine engine_;
	DataBase database_;

public:
	EngineServerHandler();

	void start();

	void stop();

	bool addPlayer(const std::string& login, const std::string& password);

	void getPlayerFleets(ndw::FleetList& _return,
	                     const ndw::Player_ID pid,
	                     const int32_t beginIndex,
	                     const int32_t endIndex,
	                     const ndw::Sort_Type::type sortType,
	                     const bool asc);

	void getPlayerPlanets(ndw::PlanetList& _return,
	                      const ndw::Player_ID pid,
	                      const int32_t beginIndex,
	                      const int32_t endIndex,
	                      const  ndw::Sort_Type::type sortType,
	                      const bool asc);

	void setPlayerFleetCode(const ndw::Player_ID pid, const std::string& code);

	void setPlayerPlanetCode(const ndw::Player_ID pid, const std::string& code);

	void setPlayerFleetBlocklyCode(const ndw::Player_ID pid, const std::string& code);

	void setPlayerPlanetBlocklyCode(const ndw::Player_ID pid, const std::string& code);

	void getPlayerFleetCode(ndw::CodeData& _return, const ndw::Player_ID pid);

	void getPlayerPlanetCode(ndw::CodeData& _return, const ndw::Player_ID pid);

	void getPlayers(std::vector<ndw::Player>& _return);

	void getPlayer(ndw::Player& _return, const ndw::Player_ID pid);

	void getPlanet(std::vector<ndw::Planet>& _return, const ndw::Coord& coord);

	void getFleet(ndw::Fleet& _return, const ndw::Fleet_ID fid);

	void logPlayer(ndw::OptionalPlayer& _return, const std::string& login, const std::string& password);

	void incrementTutoDisplayed(const ndw::Player_ID pid, const std::string& tutoName);

	void getFightReport(ndw::FightReport& _return, const int32_t id);

	void getTimeInfo(ndw::TimeInfo& _return);

	bool eraseAccount(const int32_t pid, const std::string& password);

	void getBuildingsInfo(std::vector<ndw::Building>& _return);

	void getCannonsInfo(std::vector<ndw::Cannon>& _return);

	void getShipsInfo(std::vector<ndw::Ship>& _return);
};