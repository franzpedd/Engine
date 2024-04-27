#pragma once

#include <Engine.h>
#include <Platform/Main.h>

namespace Cosmos
{
	// forward declarations
	class Player;

	class Game : public Application
	{
	public:

		// constructor
		Game(std::string path);

		// destructor
		virtual ~Game();

	public:

		// event handling
		virtual void OnEvent(Shared<Event> event);

	private:
    
		std::string mPath;
		
		std::unique_ptr<Player> mPlayer;
	};
}