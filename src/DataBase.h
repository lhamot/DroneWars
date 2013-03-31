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
	DataBase();
	~DataBase();

	Player addPlayer(std::string const& login, std::string const& password);

	boost::optional<Player> getPlayer(
	  std::string const& login, std::string const& password) const;

	Player getPlayer(Player::ID) const;

	void addEvents(std::vector<Event> const& events);

	std::vector<Event> getPlayerEvents(Player::ID pid) const;

	std::vector<Event> getPlanetEvents(Coord pcoord) const;

	std::vector<Event> getFleetEvents(Player::ID pid, Fleet::ID fid) const;

	void resetPlanetEvents(Coord pcoord);

	void removeOldEvents();
};

#endif //__DRONEWARS_DATABASE__
