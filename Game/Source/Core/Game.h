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
		virtual ~Game();

	private:
    
		std::string mPath;
	};
}