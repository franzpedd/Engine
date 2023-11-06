#include "Gizmo.h"

#include "UI/Viewport.h"

namespace Cosmos
{
	Gizmo::Gizmo(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer, Scene* scene, Camera* camera, Viewport* viewport)
		: Entity(scene), mWindow(window), mRenderer(renderer), mScene(scene), mCamera(camera), mViewport(viewport)
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
		// mouse position
		double xpos, ypos;
		mWindow->GetCursorPosition(&xpos, &ypos);

		// viewport rectangle
		ImVec2 viewportSize = mViewport->GetSize();
		ImVec2 viewportMin = mViewport->GetWindowContentRegionMin();
		ImVec2 viewportMax = mViewport->GetWindowContentRegionMax();

		// click was made inside skybox
		bool validClick = (button == BUTTON_LEFT) ? true : false;
		validClick &= xpos > viewportMin.x && xpos < viewportMax.x;
		validClick &= ypos > viewportMin.y && ypos < viewportMax.y;

		if (validClick)
		{
			// ray in 3d normalised device coordinates (must check for vulkan compliancy, since every resource uses openGL)
			float x = (2.0f * (float)xpos) / viewportSize.x - 1.0f;
			float y = 1.0f - (2.0f * (float)ypos) / viewportSize.y;
			float z = 1.0f;
			glm::vec3 rayNormalized = glm::vec3(x, y, z);
			glm::vec4 rayClip = glm::vec4(rayNormalized.x, rayNormalized.y, -1.0, 1.0);
			glm::vec4 rayEye = glm::inverse(mCamera->GetProjection()) * rayClip;
			rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

			glm::vec3 rayWorld = glm::inverse(mCamera->GetView()) * glm::vec4(rayEye.x, rayEye.y, rayEye.z, 1.0);
			rayWorld = glm::normalize(rayWorld);

			int test = 0;
		}
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