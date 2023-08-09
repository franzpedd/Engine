#include "Application.h"

#include "UI.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Application::Application()
	{
		mWindow = Window::Create("Cosmos Application", 1280, 720);
		mRenderer = Renderer::Create(mWindow);

		mUI = mRenderer->UserInterface();
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
			mRenderer->OnUpdate();
		}
	}
}