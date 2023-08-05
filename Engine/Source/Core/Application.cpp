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
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (!mWindow->ShouldQuit())
		{
			mWindow->Update();
			//mGame->Update();
			mRenderer->Update();
		}
	}
}