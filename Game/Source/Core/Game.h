#pragma once

#include <Engine.h>

#include "Entity/Player.h"

namespace Cosmos
{
	class Game : public Application
	{
	public:

		// constructor
		Game(std::string path);

		// destructor
		virtual ~Game();

	private:
    
		std::string mPath;
		
		std::unique_ptr<Player> mPlayer;
	};
}