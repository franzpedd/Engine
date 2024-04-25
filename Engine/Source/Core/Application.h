#pragma once

#include "Platform/Window.h"
#include "UI/GUI.h"
#include <chrono>
#include <filesystem>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class Camera;
	class Event;
	class Renderer;
	class Scene;
	
	class Application
	{
	public:

		// constructor
		Application();

		// destructor
		virtual ~Application();

		// returns the singleton
		static inline Application* GetInstance() { return sApplication; }

		// returns the main window
		inline std::shared_ptr<Window> GetWindow() { return mWindow; }

		// returns the main camera
		inline Shared<Camera> GetCamera() { return mCamera; }

		// returns the frames per second system
		inline Shared<FramesPerSecond> GetFPSSystem() { return mFpsSystem; }

		// returns the user interface
		inline std::shared_ptr<GUI> GetGUI() { return mUI; }

		// returns the active scene
		inline std::shared_ptr<Scene> GetActiveScene() { return mScene; }

	public:

		// initializes main loop
		void Run();

		// event handling
		void OnEvent(Shared<Event> event);

	protected:

		static Application* sApplication;
		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Scene> mScene;
		std::shared_ptr<Renderer> mRenderer;
		std::shared_ptr<GUI> mUI;
		Shared<FramesPerSecond> mFpsSystem;
		Shared<Camera> mCamera;
	};

	// creates an application object
	Application* CreateApplication();
}