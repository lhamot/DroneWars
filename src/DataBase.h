#ifndef __DRONEWARS_DATABASE__
#define __DRONEWARS_DATABASE__

#include "stdafx.h"
#include <memory>
#include "Model.h"

namespace Poco {namespace Data {class Session;}}

struct Message
{
	typedef uint32_t ID;
	ID id;
	Player::ID sender;
	Player::ID recipient;
	time_t time;
	std::string subject;
	std::string message;
	std::string senderLogin;

	Message(uint32_t id,
	        Player::ID sender,
	        Player::ID recipient,
	        time_t time,
	        std::string const& subject,
	        std::string const& message,
	        std::string const& senderLogin):
		id(id),
		sender(sender),
		recipient(recipient),
		time(time),
		subject(subject),
		message(message),
		senderLogin(senderLogin)
	{
	}
};

struct FriendshipRequests
{
	std::vector<Player> sent;
	std::vector<Player> received;
};


class DataBase
{
	std::unique_ptr<Poco::Data::Session> session_;

	void addScriptImpl(Player::ID pid,
	                   CodeData::Target target,
	                   std::string const& code);

	void addBlocklyCodeImpl(Player::ID pid,
	                        CodeData::Target target,
	                        std::string const& code);

public:
	class Exception : public std::runtime_error
	{
	public:
		Exception(std::string const& message): std::runtime_error(message) {}
	};

	DataBase();
	~DataBase();

	//***************************  Player  ************************************

	Player::ID addPlayer(std::string const& login,
	                     std::string const& password,
	                     std::vector<std::string> const& codes);

	void setPlayerMainPlanet(Player::ID, Coord mainPlanet);

	boost::optional<Player> getPlayer(
	  std::string const& login, std::string const& password) const;

	Player getPlayer(Player::ID) const;

	std::vector<Player> getPlayers() const;

	void eraseAccount(Player::ID pid);

	void updateScore(std::map<Player::ID, uint64_t> const& scoreMap);

	void updateXP(std::map<Player::ID, uint32_t> const& expMap);

	bool buySkill(Player::ID pid, int16_t skillID, size_t cost);

	//***************************  Event  *************************************

	void addEvents(std::vector<Event> const& events);

	std::vector<Event> getPlayerEvents(Player::ID pid) const;

	std::vector<Event> getPlanetEvents(Player::ID pid, Coord pcoord) const;

	std::vector<Event> getFleetEvents(Player::ID pid, Fleet::ID fid) const;

	void resetPlanetEvents(Coord pcoord);

	void removeOldEvents();

	//***************************  Script  ************************************

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

	//***************************  FightReport  *******************************

	size_t addFightReport(FightReport const& report);

	void addFightReports(std::vector<FightReport> const& reports);

	FightReport getFightReport(size_t reportID);

	//***************************  Tutos  *************************************

	void incrementTutoDisplayed(std::vector<Player::ID> const& pid,
	                            std::string const& tutoName);

	void incrementTutoDisplayed(Player::ID pid, std::string const& tutoName);

	typedef std::map<std::string, size_t> PlayerTutoMap;
	PlayerTutoMap getTutoDisplayed(Player::ID pid) const;

	std::map<Player::ID, PlayerTutoMap> getAllTutoDisplayed() const;

	//***************************  Messages  **********************************

	void addMessage(Player::ID sender,
	                Player::ID recipient,
	                std::string const& suject,
	                std::string const& message);

	std::vector<Message> getMessages(Player::ID recipient);

	void eraseMesage(Message::ID mid);

	//***************************  Friendship  ********************************

	void addFriendshipRequest(Player::ID sender, Player::ID recipient);

	void acceptFriendshipRequest(
	  Player::ID sender, Player::ID recipient, bool accept);

	void closeFriendship(Player::ID playerA, Player::ID playerB);

	std::vector<Player> getFriends(Player::ID player) const;

	FriendshipRequests getFriendshipRequest(Player::ID player) const;

	//***************************  Alliance  **********************************

	Alliance::ID addAlliance(Player::ID pid,
	                         std::string const& name,
	                         std::string const& description);

	Alliance getAlliance(Alliance::ID aid);

	void updateAlliance(Alliance const& alliance);

	void transfertAlliance(Alliance::ID aid, Player::ID pid);

	void eraseAlliance(Alliance::ID aid);

	void joinAlliance(Player::ID pid, Alliance::ID aid);

	void quitAlliance(Player::ID pid);
};

#endif //__DRONEWARS_DATABASE__
