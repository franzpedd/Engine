#include "Core/Game.h"

int main(int argc, char* argv[])
{
	std::string gameFolder = Cosmos::util::GetAssetDir();

	PROFILER_BEGIN("Startup", "Startup.json");
	Cosmos::Game* game = new Cosmos::Game(gameFolder);
	PROFILER_END();

	// testing
	game->Initialize();
	game->PostInitialize();

	PROFILER_BEGIN("Runtime", "Runtime.json");
	game->Run();
	PROFILER_END();

	PROFILER_BEGIN("End", "End.json");
	delete game;
	PROFILER_END();

	return 0;
}