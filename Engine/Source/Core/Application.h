#pragma once

#include "Util/Keycodes.h"
#include "Util/Random.h"
#include <chrono>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class Window;
	class Renderer;
	class Scene;
	class GUI;

	class Application
	{
	public:

		// constructor
		Application();

		// destructor (beware: destructor is called whenever the copy constructor is invoked, so avoid using it)
		virtual ~Application() = default;

		// returns the singleton
		static inline Application* Get() { return sApplication; }

		// returns the average fps calculated
		inline uint32_t GetCurrentAverageFPS() { return mLastFPS; }

		// returns the logic timestep to update logic
		inline float GetTimestep() { return mTimestep; }

		// returns the random number generator system
		inline Random& GetRandomNumberGenerator() { return mRandom; }

	public:

		// initializes main loop
		void Run();

	public:

		// called when a mouse was moved
		void OnMouseMove(float x, float y);

		// called when a mouse was scrolled
		void OnMouseScroll(float y);

		// called when a mouse button was pressed
		void OnMousePress(Buttoncode button);

		// called when a mouse pbutton was released
		void OnMouseRelease(Buttoncode button);

		// called when a keyboard key is pressed
		void OnKeyboardPress(Keycode key);

		// called when a keyboard key is released
		void OnKeyboardRelease(Keycode key);

	protected:

		static Application* sApplication;
		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Scene> mScene;
		std::shared_ptr<Renderer> mRenderer;
		std::shared_ptr<GUI> mUI;
		Random mRandom;

		// fps system
		std::chrono::steady_clock::time_point mStart = {};
		std::chrono::steady_clock::time_point mEnd = {};
		double mTimeDifference = 0.0f;
		float mFpsTimer = 0.0f;
		uint32_t mFrameCount = 0; // renderer current fps
		float mTimestep = 1.0f; // timestep/delta time (used to update logic)
		uint32_t mLastFPS = 0;
	};
}