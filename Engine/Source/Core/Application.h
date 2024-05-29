#pragma once

#include "Camera.h"
#include "Scene.h"
#include "Event/Event.h"
#include "Platform/Window.h"
#include "Renderer/Renderer.h"
#include "UI/GUI.h"
#include "Util/Memory.h"

namespace Cosmos
{	
	class Application
	{
	public:

		// constructor
		Application();

		// destructor
		virtual ~Application();

		// returns the singleton
		static inline Application* GetInstance() { return sApplication; }

		// returns the renderer
		static inline Shared<Renderer> GetRenderer() { return mRenderer; }

	public:

		// returns the main window
		inline Shared<Window> GetWindow() { return mWindow; }

		// returns the main camera
		inline Shared<Camera> GetCamera() { return mCamera; }

		// returns the frames per second system
		inline Shared<FramesPerSecond> GetFPSSystem() { return mFpsSystem; }

		// returns the user interface
		inline Shared<GUI> GetGUI() { return mUI; }

		// returns the active scene
		inline Scene* GetActiveScene() { return mScene; }

	public:

		// initializes main loop
		void Run();

		// event handling
		virtual void OnEvent(Shared<Event> event);

		// creates a new emtpy scene
		void CreateNewScene();

	protected:

		static Application* sApplication;
		Shared<Window> mWindow;
		static Shared<Renderer> mRenderer;
		Shared<GUI> mUI;
		Shared<FramesPerSecond> mFpsSystem;
		Shared<Camera> mCamera;

		Scene* mScene;
	};

	// creates an application object
	Application* CreateApplication();
}