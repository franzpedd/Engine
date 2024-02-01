#include "epch.h"
#include "Application.h"

#include "Scene.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "UI/GUI.h"

namespace Cosmos
{
	Application* Application::sApplication = nullptr;

	Application::Application()
	{
		PROFILER_FUNCTION();

		LOG_ASSERT(sApplication == nullptr, "Application already created");
		sApplication = this;

		// create objects
		mWindow = Window::Create("Cosmos Application", 1280, 720);
		mScene = new Scene(mWindow);
		mRenderer = Renderer::Create(mWindow, mScene);
		mUI = GUI::Create(mWindow, mRenderer);

		// connect the UI to the renderer to handle resize events
		mRenderer->ConnectUI(mUI);
		mScene->ConnectRenderer(mRenderer);
	}

	void Application::Run()
	{
		PROFILER_FUNCTION();

		mRenderer->Intialize();

		while (!mWindow->ShouldQuit())
		{
			PROFILER_SCOPE("Run-Loop");

			// starts fps system
			{
				mStart = std::chrono::high_resolution_clock::now(); // starts timer
			}
			
			// updates current tick
			{
				PROFILER_SCOPE("Run-Loop Update");
				mScene->OnUpdate(mTimeStep);		// updates scene
				mUI->OnUpdate();					// updates ui
				mRenderer->OnUpdate();				// updates renderer
				mWindow->OnUpdate();				// updates window
			}
			
			// end fps system
			{
				mEnd = std::chrono::high_resolution_clock::now();	// ends timer
				mFrames++;											// add frame to the count
			
				// calculates time taken by the renderer updating
				mTimeDifference = std::chrono::duration<double, std::milli>(mEnd - mStart).count(); 
			
				mTimeStep = (float)mTimeDifference / 1000.0f; // timestep
			
				// calculates time taken by last timestamp and renderer finished
				mFpsTimer += (float)mTimeDifference;
			
				if (mFpsTimer > 1000.0f) // greater than next frame, reset frame counting
				{
					mLastFPS = (uint32_t)((float)mFrames * (1000.0f / mFpsTimer));
					mFrames = 0;
					mFpsTimer = 0.0f;
				}
			}
		}

		// re-enable default console to catch quiting errors
		Logger::Get().UseExternalConsole(false);

		mScene->Destroy();
		mUI->Destroy();

		delete mScene;
	}

	void Application::OnMouseMove(float x, float y)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnMouseMove(x, y);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnMouseMove(x, y);
		}
	}

	void Application::OnMouseScroll(float y)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnMouseScroll(y);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnMouseScroll(y);
		}
	}

	void Application::OnMousePress(Buttoncode button)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnMousePress(button);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnMousePress(button);
		}
	}

	void Application::OnMouseRelease(Buttoncode button)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnMouseRelease(button);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnMouseRelease(button);
		}
	}

	void Application::OnKeyboardPress(Keycode key)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnKeyboardPress(key);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnKeyboardPress(key);
		}
	}

	void Application::OnKeyboardRelease(Keycode key)
	{
		for (auto& ent : mScene->Entities()->GetEntitiesVector())
		{
			ent->OnKeyboardRelease(key);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnKeyboardRelease(key);
		}
	}
}