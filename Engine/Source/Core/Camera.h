#pragma once

#include "Platform/Keycodes.h"
#include "Util/Math.h"

#include <memory>

namespace Cosmos
{
	class Camera
	{
	public:

		enum Type
		{
			LOOK_AT = 0,
			EDITOR_FLY
		};

	public:

		// constructor
		Camera();

		// destructor
		~Camera() = default;

		// returns the perspective/projection matrix, using defaults aspect ratio
		glm::mat4& GetProjection();

		// returns the view matrix
		glm::mat4& GetView();

		// returns the camera field of view
		float GetFov() const { return mFov; }

		// returns near value
		float GetNear() const { return mZnear; }

		// returns far value
		float GetFar() const { return mZfar; }

		// returns the camera eyes
		glm::vec3& GetFront() { return camFront; }

		// returns the current camera position
		glm::vec3& GetPosition() { return mPosition; }

		// returns the current camera rotation
		glm::vec3& GetRotation() { return mRotation; }

		// sets a new aspect ratio for the camera
		void SetAspectRatio(float aspect);

	public:

		// mouse was recently moved
		void OnMouseMove(float x, float y);

		// mouse was recently scrolled
		void OnMouseScroll(float y);

		// keyboard key was recrently pressed
		void OnKeyboardPress(Keycode key);

	public:

		// updates the camera
		void OnUpdate(float timestep);

		// updates the camera's view
		void UpdateViewMatrix();

		// translates the camera
		void Translate(glm::vec3 delta);

		// rotates the camera
		void Rotate(glm::vec3 delta);

	private:

		Type mType = Type::EDITOR_FLY;
		bool mFlipY = true;
		bool mShouldMove = false;
		float mPitch = 0.0f;
		float mFov = 45.0f;
		float mZnear = 0.1f;
		float mZfar = 256.0f;
		float mMovementSpeed = 1.0f;
		float mRotationSpeed = 1.0f;
		float mAspectRatio = 1.0f;

		glm::mat4 mPerspective = glm::mat4(1.0f);
		glm::mat4 mView = glm::mat4(1.0f);

		glm::vec3 camFront = glm::vec3(0.0f);
		glm::vec3 mRotation = glm::vec3(0.0f);
		glm::vec3 mPosition = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 mScale = glm::vec3(1.0f);
	};
}