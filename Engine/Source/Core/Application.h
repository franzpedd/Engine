#pragma once

#include "Platform/Keycodes.h"
#include <chrono>
#include <filesystem>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class GUI;
	class Renderer;
	class Scene;
	class Window;
	
	class Application
	{
	public:

		// constructor
		Application();

		// destructor
		virtual ~Application();

		// returns the singleton
		static inline Application* Get() { return sApplication; }

		// returns the average fps calculated
		inline uint32_t GetAverageFPS() const { return mLastFPS; }

		// returns the logic timestep to update logic
		inline double GetTimeStep() const { return mTimeStep; }

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

	private:

		// called after everyting on the tick was updated, to retrieve fps count
		void FinishFrameCalculation();

	protected:

		static Application* sApplication;
		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Scene> mScene;
		std::shared_ptr<Renderer> mRenderer;
		std::shared_ptr<GUI> mUI;
		
		// fps system
		std::chrono::steady_clock::time_point mStart = {};
		std::chrono::steady_clock::time_point mEnd = {};
		double mTimeDifference = 0.0f;
		float mFpsTimer = 0.0f;
		uint32_t mFrames = 0; // average fps
		float mTimeStep = 1.0f; // timestep/delta time (used to update logic)
		uint32_t mLastFPS = 0;
	};

	// creates an application object
	Application* CreateApplication();
}