#include "Game.h"

#include "Entity/Player.h"

namespace Cosmos
{
	// defined on engine Platform/Main.h
	Application* CreateApplication()
	{
		return new Game(GetAssetDir());
	}

	Game::Game(std::string path)
		: mPath(path)
	{
		DataFile scene = {};
		DataFile::Read(scene, GetAssetSubDir("Project.cosmos"));
		mScene->Deserialize(scene);

		mPlayer = std::make_unique<Player>();
	}

	Game::~Game()
	{

	}

	void Game::OnEvent(Shared<Event> event)
	{
		Application::OnEvent(event);

		if (event->GetType() == EventType::KeyboardPress)
		{
			auto camera = Application::GetInstance()->GetCamera();
			auto castedEvent = std::dynamic_pointer_cast<KeyboardPressEvent>(event);
			Keycode key = castedEvent->GetKeycode();

			// toggle editor viewport camera, move to viewport
			if (key == KEY_Z)
			{
				if (camera->CanMove() && camera->GetType() == Camera::Type::EDITOR_FLY)
				{
					camera->SetMove(false);
					Application::GetInstance()->GetWindow()->ToggleCursor(false);
					UIToggleCursor(false);
				}

				else if (!camera->CanMove() && camera->GetType() == Camera::Type::EDITOR_FLY)
				{
					camera->SetMove(true);
					Application::GetInstance()->GetWindow()->ToggleCursor(true);
					UIToggleCursor(true);
				}
			}
		}
	}
}