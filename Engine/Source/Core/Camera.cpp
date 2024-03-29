#include "epch.h"
#include "Camera.h"

#include "Platform/Window.h"
#include "UI/GUI.h"

namespace Cosmos
{
	Camera::Camera()
	{
		// initial camera aspect ratio
		mAspectRatio = Window::Get()->GetAspectRatio();

		// update initial position
		UpdateViewMatrix();
	}

	glm::mat4& Camera::GetProjection()
	{
		mPerspective = glm::perspectiveRH(glm::radians(mFov), mAspectRatio, mZnear, mZfar);

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

	void Camera::SetAspectRatio(float aspect)
	{
		mAspectRatio = aspect;
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
	}

	void Camera::OnMouseScroll(float y)
	{
		// doing nothing right now
	}

	void Camera::OnKeyboardPress(Keycode key)
	{
		if (!Window::Get()->Hovered()) // disable outside window commands
			return;

		if (key == KEY_Z && mShouldMove && mType == EDITOR_FLY)
		{
			mShouldMove = false;
			Window::Get()->ToogleCursorMode(false);
			ToogleMouseCursor(false);
		}

		else if (key == KEY_Z && !mShouldMove && mType == EDITOR_FLY)
		{
			mShouldMove = true;
			Window::Get()->ToogleCursorMode(true);
			ToogleMouseCursor(true);
		}
	}

	void Camera::OnUpdate(float timestep)
	{
		if (!mShouldMove) return;

		camFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
		camFront.y = sin(glm::radians(mRotation.x));
		camFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));
		camFront = glm::normalize(camFront);

		float moveSpeed = timestep * mMovementSpeed;

		if (Window::Get()->IsKeyDown(KEY_W)) mPosition += camFront * moveSpeed;
		if (Window::Get()->IsKeyDown(KEY_S)) mPosition -= camFront * moveSpeed;
		if (Window::Get()->IsKeyDown(KEY_A)) mPosition -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
		if (Window::Get()->IsKeyDown(KEY_D)) mPosition += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

		UpdateViewMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotMat = glm::mat4(1.0f);
		glm::mat4 translateMat = glm::mat4(1.0f);
		glm::mat4 scaleMat = glm::mat4(1.0f);
		
		rotMat = glm::rotate(rotMat, glm::radians(mRotation.x * (mFlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotMat = glm::rotate(rotMat, glm::radians(mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotMat = glm::rotate(rotMat, glm::radians(mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		
		glm::vec3 translation = mPosition;
		glm::vec3 scale = mScale;

		if (mFlipY)
		{
			translation.y *= -1.0f;
		}
		
		translateMat = glm::translate(glm::mat4(1.0f), translation);
		scaleMat = glm::scale(glm::mat4(1.0f), scale);
		
		if (mType == Type::EDITOR_FLY) mView = scaleMat * rotMat * translateMat;
		else mView = scaleMat * translateMat * rotMat;
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