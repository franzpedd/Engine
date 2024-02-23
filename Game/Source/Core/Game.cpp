#include "Game.h"

namespace Cosmos
{
	Game::Game(std::string path)
		: mPath(path)
	{

	}

	void Game::PostInitialize()
	{
		// load scene
		DataFile scene = {};
		DataFile::Read(scene, util::GetAssetSubDir("Project.cosmos"));
		mScene->Deserialize(scene);
	}
}