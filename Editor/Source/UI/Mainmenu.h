#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Mainmenu : public UIElement
	{
	public:

		// constructor
		Mainmenu(Camera& camera);

		// destructor
		virtual ~Mainmenu();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnResize() override;

	private:

		// drawing and logic of main menu
		void DisplayMainMenu();

	private:

		Camera& mCamera;
	};
}