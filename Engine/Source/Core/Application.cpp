#include "Application.h"

#include "Scene.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "UI/GUI.h"
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
		mUI = GUI::Create(mWindow, mRenderer);

		// connect the UI to the renderer to handle resize events
		mRenderer->ConnectUI(mUI);

		// setup number generator seed
		mRandom.Seed();
	}

	void Application::Run()
	{
		while (!mWindow->ShouldQuit())
		{
			// starts fps system
			{
				mStart = std::chrono::high_resolution_clock::now(); // starts timer
			}

			// updates current tick
			{
				mScene->OnUpdate(mTimestep);		// updates scene
				mUI->OnUpdate();					// updates ui
				mRenderer->OnUpdate();				// updates renderer
				mWindow->OnUpdate();				// updates window
			}
			
			// end fps system
			{
				mEnd = std::chrono::high_resolution_clock::now();	// ends timer
				mFrameCount++;										// add frame to the count

				// calculates time taken by the renderer updating
				mTimeDifference = std::chrono::duration<double, std::milli>(mEnd - mStart).count(); 

				mTimestep = (float)mTimeDifference / 1000.0f; // timestep

				// calculates time taken by last timestamp and renderer finished
				mFpsTimer += (float)mTimeDifference;

				if (mFpsTimer > 1000.0f) // greater than next frame, reset frame counting
				{
					mLastFPS = (uint32_t)((float)mFrameCount * (1000.0f / mFpsTimer));
					mFrameCount = 0;
					mFpsTimer = 0.0f;
				}
			}
		}

		mScene->Destroy();
		mUI->Destroy();
	}

	void Application::OnMouseMove(float x, float y)
	{
		for (auto& ent : mScene->Entities())
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
		for (auto& ent : mScene->Entities())
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
		for (auto& ent : mScene->Entities())
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
		for (auto& ent : mScene->Entities())
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
		for (auto& ent : mScene->Entities())
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
		for (auto& ent : mScene->Entities())
		{
			ent->OnKeyboardRelease(key);
		}

		for (auto& widget : mUI->Widgets())
		{
			widget->OnKeyboardRelease(key);
		}
	}
}