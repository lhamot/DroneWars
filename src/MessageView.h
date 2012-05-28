#ifndef __BTA_MESSAGEVIEW__
#define __BTA_MESSAGEVIEW__

#pragma warning(push)
#pragma warning(disable:4127 4251 4231 4800 4512 4275 4505)
#include <Wt/WContainerWidget>
#pragma warning(pop)

#include "Engine.h"

class MessageView : public Wt::WContainerWidget
{
public:
	MessageView(
	  WContainerWidget* parent,
	  Engine& eng,
	  Player::ID playerID,
	  Event::ID eventID);

private:
	void renderFightReport(size_t id);

	Engine& engine_;
	Player::ID playerID_;
	Event::ID eventID_;
};

#endif