#include "epch.h"
#include "Camera.h"

#include "Application.h"
#include "Event/InputEvent.h"
#include "UI/GUI.h"

namespace Cosmos
{
	Camera::Camera()
	{
		// initial camera aspect ratio
		mAspectRatio = Application::GetInstance()->GetWindow()->GetAspectRatio();

		// update initial position
		UpdateViewMatrix();
	}

	glm::mat4& Camera::GetProjectionRef()
	{
		mPerspective = glm::perspectiveRH(glm::radians(mFov), mAspectRatio, mZnear, mZfar);

		if (mFlipY)
		{
			mPerspective[1][1] *= -1.0f;
		}

		return mPerspective;
	}

	glm::mat4& Camera::GetViewRef()
	{
		return mView;
	}

	void Camera::OnUpdate(float timestep)
	{
		if (!mShouldMove)
			return;

		mFront.x = -cos(glm::radians(mRotation.x)) * sin(glm::radians(mRotation.y));
		mFront.y = sin(glm::radians(mRotation.x));
		mFront.z = cos(glm::radians(mRotation.x)) * cos(glm::radians(mRotation.y));

		float moveSpeed = timestep * mMovementSpeed;

		if (mMovingForward) mPosition += mFront * moveSpeed;
		if (mMovingBackward) mPosition -= mFront * moveSpeed;
		if (mMovingLeft) mPosition -= glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
		if (mMovingRight) mPosition += glm::normalize(glm::cross(mFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

		UpdateViewMatrix();
	}

	void Camera::OnEvent(Shared<Event> event)
	{
		switch (event->GetType())
		{
			case EventType::MouseMove:
			{
				if (!mShouldMove) return;

				// avoid scene flip
				if (mRotation.x >= 89.0f) mRotation.x = 89.0f;
				if (mRotation.x <= -89.0f) mRotation.x = -89.0f;

				// reset rotation on 360 degrees
				if (mRotation.x >= 360.0f) mRotation.x = 0.0f;
				if (mRotation.x <= -360.0f) mRotation.x = 0.0f;
				if (mRotation.y >= 360.0f) mRotation.y = 0.0f;
				if (mRotation.y <= -360.0f) mRotation.y = 0.0f;

				float x = (float)std::dynamic_pointer_cast<MouseMoveEvent>(event)->GetXOffset();
				float y = (float)std::dynamic_pointer_cast<MouseMoveEvent>(event)->GetYOffset();

				Rotate(glm::vec3(-y * mRotationSpeed * 0.5f, x * mRotationSpeed * 0.5f, 0.0f));
				break;
			}

			case EventType::MouseWheel:
			{
				if (!mShouldMove) return;

				float delta = (float)std::dynamic_pointer_cast<MouseWheelEvent>(event)->GetDelta();

				mPosition += delta * mMovementSpeed;
				break;
			}

			case EventType::KeyboardPress:
			{
				auto castedEvent = std::dynamic_pointer_cast<KeyboardPressEvent>(event);
				Keycode key = castedEvent->GetKeycode();

				if (key == KEY_W && mShouldMove) mMovingForward = true;
				if (key == KEY_S && mShouldMove) mMovingBackward = true;
				if (key == KEY_A && mShouldMove) mMovingLeft = true;
				if (key == KEY_D && mShouldMove) mMovingRight = true;

				break;
			}

			case EventType::KeyboardRelease:
			{
				auto castedEvent = std::dynamic_pointer_cast<KeyboardReleaseEvent>(event);
				Keycode key = castedEvent->GetKeycode();

				if (key == KEY_W && mShouldMove) mMovingForward = false;
				if (key == KEY_S && mShouldMove) mMovingBackward = false;
				if (key == KEY_A && mShouldMove) mMovingLeft = false;
				if (key == KEY_D && mShouldMove) mMovingRight = false;

				break;
			}
		}
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
		
		if (mType == Type::FREE_LOOK) mView = scaleMat * rotMat * translateMat;
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