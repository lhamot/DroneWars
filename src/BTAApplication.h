#include "Player.h"

class Engine;

class BTAApplication : public Wt::WApplication
{
public:
	BTAApplication(const Wt::WEnvironment& env, Engine& engine);

private:
	void onPlayerLogin(Player::ID pid);

	void handleInternalPath(const std::string& internalPath);

	void notify(Wt::WEvent& event);

	Engine& engine_;
	Player::ID playerID_;
};

Wt::WApplication* createApplication(const Wt::WEnvironment& env, Engine& engine);