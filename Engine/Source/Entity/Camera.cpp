#include "Camera.h"

#include "UI/GUI.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Camera::Camera(std::shared_ptr<Window>& window, Scene* scene)
		: Entity(scene), mWindow(window)
	{
		// update initial position
		UpdateViewMatrix();

		// initial camera aspect ratio
		mAspectRatio = mWindow->GetAspectRatio();

		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Make camera an entity");
	}

	glm::mat4& Camera::GetProjection()
	{
		mPerspective = glm::perspective(mFov, mAspectRatio, mZnear, mZfar);

		if (mFlipY)
		{
			mPerspective[1][1] *= -1.0f;
		}

		return mPerspective;
	}

	glm::mat4& Camera::GetView()
	{
		return mView;
	}

	void Camera::OnMouseMove(float x, float y)
	{
		if (!mShouldMove) return;

		// disables scene flipping 
		if (mRotation.x >= 89.0f) mRotation.x = 89.0f;
		if (mRotation.x <= -89.0f) mRotation.x = -89.0f;

		// fix rotation
		if (mRotation.x >= 360.0f) mRotation.x = 0.0f;
		if (mRotation.x <= -360.0f) mRotation.x = 0.0f;
		if (mRotation.y >= 360.0f) mRotation.y = 0.0f;
		if (mRotation.y <= -360.0f) mRotation.y = 0.0f;

		Rotate(glm::vec3(-y * mRotationSpeed * 0.5f, x * mRotationSpeed * 0.5f, 0.0f));

		// axis move and zooming
		// Translate(glm::vec3(-0.0f, 0.0f, y * .005f));
		// Translate(glm::vec3(-x * 0.005f, -y * 0.005f, 0.0f));
	}

	void Camera::OnMouseScroll(float y)
	{
		if (!mShouldMove) return;

		Translate(glm::vec3(0.0f, 0.0f, y * 0.005f));
	}

	void Camera::OnKeyboardPress(Keycode key)
	{
		if (!mWindow->Hovered()) // disable outside window commands
		{
			return;
		}

		if (key == KEY_Z && mShouldMove && mType == EDITOR_FLY)
		{
			mShouldMove = false;
			mWindow->ToogleCursorMode(false);
			ToogleMouseCursor(false);
		}

		else if (key == KEY_Z && !mShouldMove && mType == EDITOR_FLY)
		{
			mShouldMove = true;
			mWindow->ToogleCursorMode(true);
			ToogleMouseCursor(true);
		}
	}

	void Camera::OnUpdate(float timestep)
	{
		if (!mShouldMove) return;

		glm::vec3 camFront;
		camFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
		camFront.y = sin(glm::radians(mRotation.x));
		camFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));
		camFront = glm::normalize(camFront);

		float moveSpeed = timestep * mMovementSpeed;

		if (mWindow->IsKeyDown(KEY_W)) mPosition += camFront * moveSpeed;
		if (mWindow->IsKeyDown(KEY_S)) mPosition -= camFront * moveSpeed;
		if (mWindow->IsKeyDown(KEY_A)) mPosition -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
		if (mWindow->IsKeyDown(KEY_D)) mPosition += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

		UpdateViewMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;
		
		rotM = glm::rotate(rotM, glm::radians(mRotation.x * (mFlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		
		glm::vec3 translation = mPosition;
		if (mFlipY) translation.y *= -1.0f;
		
		transM = glm::translate(glm::mat4(1.0f), translation);
		
		if (mType == Type::EDITOR_FLY) mView = rotM * transM;
		else mView = transM * rotM;
	}

	void Camera::Translate(glm::vec3 delta)
	{
		mPosition += delta;
		UpdateViewMatrix();
	}

	void Camera::Rotate(glm::vec3 delta)
	{
		mRotation += delta;
		UpdateViewMatrix();
	}
}