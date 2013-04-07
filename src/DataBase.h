#ifndef __DRONEWARS_DATABASE__
#define __DRONEWARS_DATABASE__

#include "stdafx.h"
#include <memory>
#include "Model.h"

namespace Poco {namespace Data {class Session;}}

class DataBase
{
	std::unique_ptr<Poco::Data::Session> session_;
public:
	class Exception : public std::runtime_error
	{
	public:
		Exception(std::string const& message): std::runtime_error(message) {}
	};

	DataBase();
	~DataBase();

	Player::ID addPlayer(std::string const& login, std::string const& password);

	void setPlayerMainPlanet(Player::ID, Coord mainPlanet);

	boost::optional<Player> getPlayer(
	  std::string const& login, std::string const& password) const;

	Player getPlayer(Player::ID) const;

	std::vector<Player> getPlayers() const;

	void addEvents(std::vector<Event> const& events);

	std::vector<Event> getPlayerEvents(Player::ID pid) const;

	std::vector<Event> getPlanetEvents(Player::ID pid, Coord pcoord) const;

	std::vector<Event> getFleetEvents(Player::ID pid, Fleet::ID fid) const;

	void resetPlanetEvents(Coord pcoord);

	void removeOldEvents();

	size_t addScript(Player::ID pid,
	                 CodeData::Target target,
	                 std::string const& code);

	size_t addBlocklyCode(Player::ID pid,
	                      CodeData::Target target,
	                      std::string const& code);

	CodeData getPlayerCode(Player::ID pid, CodeData::Target) const;

	struct CodeError
	{
		size_t codeDataId;
		std::string message;
	};
	void addCodeErrors(std::vector<CodeError> const& errors);

	void addCodeError(size_t scriptId, std::string const& message);

	size_t addFightReport(FightReport const& report);

	void addFightReports(std::vector<FightReport> const& reports);

	FightReport getFightReport(size_t reportID);

	void eraseAccount(Player::ID pid);

	void incrementTutoDisplayed(std::vector<Player::ID> const& pid,
	                            std::string const& tutoName);

	void incrementTutoDisplayed(Player::ID pid, std::string const& tutoName);

	typedef std::map<std::string, size_t> PlayerTutoMap;
	PlayerTutoMap getTutoDisplayed(Player::ID pid) const;

	std::map<Player::ID, PlayerTutoMap> getAllTutoDisplayed() const;

	void updateScore(std::map<Player::ID, uint64_t> const& scoreMap);
};

#endif //__DRONEWARS_DATABASE__
