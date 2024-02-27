#include "Game.h"

namespace Cosmos
{
	Game::Game(std::string path)
		: mPath(path)
	{
		DataFile scene = {};
		DataFile::Read(scene, util::GetAssetSubDir("Project.cosmos"));
		mScene->Deserialize(scene);

		mPlayer = std::make_unique<Player>();
	}

	Game::~Game()
	{

	}
}