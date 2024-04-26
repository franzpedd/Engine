#include "epch.h"
#include "Application.h"

#include "Sound/Listener.h"
#include "Thread/Pool.h"
#include "Util/FileSystem.h"

namespace Cosmos
{
	Application* Application::sApplication = nullptr;

	Application::Application()
	{
		PROFILER_FUNCTION();

		LOG_ASSERT(sApplication == nullptr, "Application already created");
		sApplication = this;

		// create singleton objects
		sound::Listener::GetInstance();
		thread::PoolManager::GetInstance();

		// create shared objects
		mFpsSystem = CreateShared<FramesPerSecond>();
		mWindow = CreateShared<Window>("Cosmos Application", 1280, 720);
		mCamera = CreateShared<Camera>();
		mRenderer = Renderer::Create();
		mScene = CreateShared<Scene>(mRenderer);
		mUI = CreateShared<GUI>(mRenderer);
	}

	Application::~Application()
	{

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