#include "Gizmo.h"

namespace Cosmos
{
	Gizmo::Gizmo(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer, Camera* camera)
		: Entity("Gizmo"), mWindow(window), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Gizmo";
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

		if (key == Keycode::KEY_E && mWindow->IsKeyDown(Keycode::KEY_LEFT_SHIFT))
		{
			LOG_TO_TERMINAL(Logger::Severity::Trace, "Scale mode");
		}
		
		if (key == Keycode::KEY_T && mWindow->IsKeyDown(Keycode::KEY_LEFT_SHIFT))
		{
			LOG_TO_TERMINAL(Logger::Severity::Trace, "Translate mode");
			return;
		}
		
		if (key == Keycode::KEY_R && mWindow->IsKeyDown(Keycode::KEY_LEFT_SHIFT))
		{
			LOG_TO_TERMINAL(Logger::Severity::Trace, "Rotate mode");
		}
	}

	void Gizmo::OnKeyboardRelease(Keycode key)
	{
	}
}