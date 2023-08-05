#pragma once

#include <memory>

namespace Cosmos
{
	// forward declaration
	class Window;
	class Renderer;
	class UI;

	class Application
	{
	public:

		// constructor
		Application();

		// destructor
		~Application();

		// initializes main loop
		void Run();

	private:

		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Renderer> mRenderer;
	};
}