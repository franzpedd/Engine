#include "Core/Game.h"

int main(int argc, char* argv[])
{
	PROFILER_BEGIN("Startup", "Startup.json");
	Cosmos::Game* game = new Cosmos::Game();
	PROFILER_END();

	PROFILER_BEGIN("Runtime", "Runtime.json");
	game->Run();
	PROFILER_END();

	PROFILER_BEGIN("End", "End.json");
	delete game;
	PROFILER_END();

	return 0;
}