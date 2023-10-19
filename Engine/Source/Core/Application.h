#pragma once

#include "Util/Keycodes.h"
#include "Util/Timestep.h"
#include <chrono>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class Window;
	class Renderer;
	class Scene;
	class UICore;

	class Application
	{
	public:

		// constructor
		Application();

		// destructor (beware: destructor is called whenever the copy constructor is invoked, so avoid using it)
		virtual ~Application() = default;

		// returns the singleton
		static inline Application* Get() { return sApplication; }

	public:

		// initializes main loop
		void Run();

		// updates the application client
		virtual void OnUpdate(Timestep ts) = 0;

	public:

		// called when a mouse was moved
		virtual void OnMouseMove(float xPos, float yPos, float xOffset, float yOffset) = 0;

		// called when a mouse was scrolled
		virtual void OnMouseScroll(float yOffset) = 0;

		// called when a mouse button was pressed
		virtual void OnMousePress(Buttoncode button) = 0;

		// called when a mouse pbutton was released
		virtual void OnMouseRelease(Buttoncode button) = 0;

		// called when a keyboard key is pressed
		virtual void OnKeyboardPress(Keycode key) = 0;

		// called when a keyboard key is released
		virtual void OnKeyboardRelease(Keycode key) = 0;

	protected:

		static Application* sApplication;
		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Scene> mScene;
		std::shared_ptr<Renderer> mRenderer;
		std::shared_ptr<UICore> mUI;
		Timestep mTs;

		float mLastFrameTime = 0.0f;
	};
}