#include "epch.h"
#include "Application.h"

#include "Camera.h"
#include "Scene.h"

#include "Event/Event.h"
#include "Renderer/Renderer.h"
#include "Sound/Listener.h"
#include "Thread/Pool.h"
#include "Util/FileSystem.h"

#include "Entity/Entity.h"

namespace Cosmos
{
	Application* Application::sApplication = nullptr;

	Application::Application()
	{
		PROFILER_FUNCTION();

		LOG_ASSERT(sApplication == nullptr, "Application already created");
		sApplication = this;

		// create objects
		sound::Listener::GetInstance();
		thread::PoolManager::GetInstance();

		mFpsSystem = CreateShared<FramesPerSecond>();
		mWindow = CreateShared<Window>("Cosmos Application", 1280, 720);
		mCamera = CreateShared<Camera>();
		mRenderer = Renderer::Create();
		mScene = CreateShared<Scene>(mRenderer);
		mUI = CreateShared<GUI>(mRenderer);
	}

	Application::~Application()
	{
		mRenderer->OnTerminate();
	}

	void Application::Run()
	{
		PROFILER_FUNCTION();

		while (!mWindow->ShouldQuit())
		{
			PROFILER_SCOPE("Run-Loop");

			// starts fps calculation
			mFpsSystem->StartFrame();
			
			// updates current tick
			mWindow->OnUpdate();
			mCamera->OnUpdate(mFpsSystem->GetTimestep());
			mScene->OnUpdate(mFpsSystem->GetTimestep());
			mUI->OnUpdate();
			mRenderer->OnUpdate();
			
			// ends fps calculation
			mFpsSystem->EndFrame();
		}
	}

	void Application::OnEvent(Shared<Event> event)
	{
		mCamera->OnEvent(event);
		mScene->OnEvent(event);
		mUI->OnEvent(event);
	}
}