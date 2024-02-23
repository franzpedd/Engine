#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Game : public Application
	{
	public:

		// constructor
		Game(std::string path);

		// destructor
		virtual ~Game() = default;

		// testing
		void PostInitialize();

	private:
    
		std::string mPath;
	};
}