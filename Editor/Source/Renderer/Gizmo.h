#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Gizmo : public Entity
	{
	public:

		// constructor
		Gizmo(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer, Camera* camera);

		// destructor
		~Gizmo() = default;

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnRenderDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDestroy() override;

	public:

		// called when a mouse button was pressed
		void OnMousePress(Buttoncode button) override;

		// called when a mouse button was released
		void OnMouseRelease(Buttoncode button) override;

		// called when a keyboard key is pressed
		void OnKeyboardPress(Keycode key) override;

		// called when a keyboard key is released
		void OnKeyboardRelease(Keycode key) override;

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<Renderer>& mRenderer;
		Camera* mCamera;
	};

}