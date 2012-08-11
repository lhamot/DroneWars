#ifndef __BTA_MESSAGEVIEW__
#define __BTA_MESSAGEVIEW__

#include "stdafx.h"
#include "Model.h"


class Engine;

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