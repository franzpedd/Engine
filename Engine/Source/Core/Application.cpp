#include "Application.h"

#include "Scene.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "UI/UICore.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Application* Application::sApplication = nullptr;

	Application::Application()
	{
		LOG_ASSERT(sApplication == nullptr, "Application already created");
		sApplication = this;

		// create objects
		mWindow = Window::Create("Cosmos Application", 1280, 720);
		mScene = Scene::Create(mWindow);
		mRenderer = Renderer::Create(mWindow, mScene);
		mUI = UICore::Create(mWindow, mRenderer);

		// connect the UI to the renderer to handle resize events
		mRenderer->ConnectUI(mUI);
	}

	void Application::Run()
	{
		while (!mWindow->ShouldQuit())
		{
			// start clock
			float time = (float)mWindow->GetTime();
			mTs = time - mLastFrameTime;
			mLastFrameTime = time;

			mScene->OnUpdate(mTs);			// scene logic
			mUI->OnUpdate();				// ui logic
			mRenderer->OnUpdate();			// render frame
			mWindow->OnUpdate();			// input events
		}

		mScene->Destroy();
	}
}