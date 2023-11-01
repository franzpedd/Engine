#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Gizmo : public Entity
	{
	public:

		enum Operation : uint16_t
		{
			TRANSLATE = 0,
			ROTATE,
			SCALE,
			UNIVERSAL
		};

	public:

		// constructor
		Gizmo(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer, Camera* camera);

		// destructor
		~Gizmo() = default;

	public:

		// draws the entity (leave empty if not required)
		virtual void OnRenderDraw() override;

		// updates the entity (leave empty if not required)
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

		// creates all resources needed for gizmos
		void CreateResources();

		// edits the transformation of a given object
		void EditTransform(Entity& ent);

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<Renderer>& mRenderer;
		Camera* mCamera;

		std::shared_ptr<VKShader> mPickVertexShader;
		std::shared_ptr<VKShader> mPickFragmentShader;

		Operation mCurrentOperation = TRANSLATE;
	};
}