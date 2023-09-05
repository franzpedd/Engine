#include "Application.h"

#include "Scene.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "UI/UICore.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Application::Application()
	{
		// create objects
		mWindow = Window::Create("Cosmos Application", 1280, 720);
		mRenderer = Renderer::Create(mWindow);
		mScene = Scene::Create(mRenderer);
		mUI = UICore::Create(mWindow, mRenderer);

		// connect the UI to the renderer to handle resize events
		mRenderer->ConnectUI(mUI);
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (!mWindow->ShouldQuit())
		{
			mWindow->OnUpdate();
			//mGame->Update();
			mUI->OnUpdate();
			mRenderer->OnUpdate();
		}
	}
}