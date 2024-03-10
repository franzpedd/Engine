#include "Game.h"

#include "Entity/Player.h"

namespace Cosmos
{
	// defined on engine Platform/Main.h
	Application* CreateApplication()
	{
		return new Game(util::GetAssetDir());
	}

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