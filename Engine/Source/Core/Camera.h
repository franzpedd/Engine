#pragma once

#include "Platform/Window.h"
#include "UI/UIDrawable.h"
#include "Util/Keycodes.h"
#include "Util/Math.h"
#include "Util/Timestep.h"
#include <memory>

namespace Cosmos
{
	class Camera
	{
	public:

		enum Type
		{
			LOOK_AT = 0,
			FIRST_PERSON
		};

	public:

		// constructor
		Camera(std::shared_ptr<Window>& window);

		// destructor
		~Camera() = default;

		// returns the perspective matrix
		glm::mat4& GetProjection();

		// returns the view matrix
		glm::mat4& GetView();

	public:

		// mouse was recently moved
		void OnMouseMove(float xOffset, float yOffset);

		// mouse was recently scrolled
		void OnMouseScroll(float yOffset);

		// keyboard key was recrently pressed
		void OnKeyboardPress(Keycode key);

	public:

		// updates the camera
		void OnUpdate(Timestep ts);

		// updates the camera's view
		void UpdateViewMatrix();

		// translates the camera
		void Translate(glm::vec3 delta);

		// rotates the camera
		void Rotate(glm::vec3 delta);

	private:

		std::shared_ptr<Window>& mWindow;

		Type mType = Type::FIRST_PERSON;
		bool mFlipY = true;
		bool mShouldMove = false;
		float mFov = 45.0f;
		float mZnear = 0.1f;
		float mZfar = 256.0f;
		float mMovementSpeed = 1.0f;
		float mRotationSpeed = 1.0f;
		glm::mat4 mPerspective = glm::mat4();
		glm::mat4 mView = glm::mat4();
		glm::vec3 mRotation = glm::vec3();
		glm::vec3 mPosition = glm::vec3();
	};
}