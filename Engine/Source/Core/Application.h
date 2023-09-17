#pragma once

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

		// destructor
		~Application();

		// initializes main loop
		void Run();

	protected:

		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Scene> mScene;
		std::shared_ptr<Renderer> mRenderer;
		std::shared_ptr<UICore> mUI;
	};
}