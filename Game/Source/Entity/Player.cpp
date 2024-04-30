#include "Player.h"

namespace Cosmos
{
	Player::Player(Entity* entity)
		: NativeScript(entity, "Player")
	{
		LOG_TO_TERMINAL(Logger::Trace, "Constructing Player");
	}

	Player::~Player()
	{
		LOG_TO_TERMINAL(Logger::Trace, "Destroying player");
	}

	void Player::OnUpdate(float timestep)
	{
		LOG_TO_TERMINAL(Logger::Trace, "Updating player");
	}
}