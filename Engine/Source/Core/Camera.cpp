#include "Camera.h"

#include "Util/Logger.h"

namespace Cosmos
{
	Camera::Camera(std::shared_ptr<Window>& window)
		: mWindow(window)
	{
	}

	glm::mat4& Camera::GetProjection()
	{
		int width;
		int height;
		mWindow->GetFramebufferSize(&width, &height);
		mPerspective = glm::perspective(mFov, (float)width / (float)height, mZnear, mZfar);

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

	void Camera::OnMouseMove(float xOffset, float yOffset)
	{
		if (!mShouldMove) return;

		Rotate(glm::vec3(-yOffset * mRotationSpeed * 0.5f, xOffset * mRotationSpeed * 0.5f, 0.0f));

		// axis move and zooming
		// Translate(glm::vec3(-0.0f, 0.0f, yOffset * .005f));
		// Translate(glm::vec3(-xOffset * 0.005f, -yOffset * 0.005f, 0.0f));
	}

	void Camera::OnMouseScroll(float yOffset)
	{
		if (!mShouldMove) return;

		Translate(glm::vec3(0.0f, 0.0f, yOffset * 0.005f));
	}

	void Camera::OnKeyboardPress(Keycode key)
	{
		if (key == KEY_Z && mShouldMove)
		{
			mShouldMove = false;
			mWindow->ToogleCursorMode(false);
			ui::ToogleMouseCursor(false);
		}

		else if (key == KEY_Z && !mShouldMove)
		{
			mShouldMove = true;
			mWindow->ToogleCursorMode(true);
			ui::ToogleMouseCursor(true);
		}
	}

	void Camera::OnUpdate(Timestep ts)
	{
		if (!mShouldMove) return;

		glm::vec3 camFront;
		camFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
		camFront.y = sin(glm::radians(mRotation.x));
		camFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));
		camFront = glm::normalize(camFront);

		float moveSpeed = ts * mMovementSpeed;

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
		
		if (mType == Type::FIRST_PERSON) mView = rotM * transM;
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