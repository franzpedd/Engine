#include "Gizmo.h"

namespace Cosmos
{
	Gizmo::Gizmo(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer, Camera* camera)
		: Entity("Gizmo"), mWindow(window), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Gizmo";

		CreateResources();
	}

	void Gizmo::OnRenderDraw()
	{
		
	}

	void Gizmo::OnUpdate(float timestep)
	{
	}

	void Gizmo::OnDestroy()
	{

	}

	void Gizmo::OnMousePress(Buttoncode button)
	{
		// START MOUSE PICKING
	}

	void Gizmo::OnMouseRelease(Buttoncode button)
	{
	}

	void Gizmo::OnKeyboardPress(Keycode key)
	{
		if (!mWindow->Hovered()) // disable outside window commands
		{
			return;
		}

		bool shift = mWindow->IsKeyDown(Keycode::KEY_LEFT_SHIFT);

		if (shift)
		{
			switch (key)
			{
			case Cosmos::KEY_Y:
				mCurrentOperation = SCALE;
				break;
			case Cosmos::KEY_T:
				mCurrentOperation = TRANSLATE;
				break;
			case Cosmos::KEY_R:
				mCurrentOperation = ROTATE;
				break;
			case Cosmos::KEY_U:
				mCurrentOperation = UNIVERSAL;
				break;
			default:
				break;
			}
		}
	}

	void Gizmo::OnKeyboardRelease(Keycode key)
	{
	}

	void Gizmo::CreateResources()
	{

	}

	void Gizmo::EditTransform(Entity& ent)
	{

	}
}