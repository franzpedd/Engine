#pragma once

#include "Core/Defines.h"
#include "Core/Input.h"

#define GLM_FORCE_RADIAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Cosmos
{
	class Camera
	{
	public:

		enum Type
		{
			LOOK_AT,
			FIRST_PERSON
		};

		struct Specification
		{
			Type Type = Type::LOOK_AT;
			glm::vec3 Rotation = glm::vec3();
			glm::vec3 Position = glm::vec3();
			f32 RotationSpeed = 1.0f;
			f32 MovementSpeed = 1.0f;
			bool Updated = false;
			glm::mat4 Perspective = glm::mat4();
			glm::mat4 View = glm::mat4();
			f32 FieldOfView = 0.0f;
			f32 ZNearClip = 0.0f;
			f32 ZFarClip = 0.0f;
		};

	public:

		// constructor
		Camera();

		// destructor
		~Camera();

		// returns the camera's private members
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// updates the camera
		void Update(f32 deltaTime);

		// updates the view matrix
		void UpdateViewMatrix();

		// returns if camera is moving
		bool IsMoving();

		// sets a new camera perspective
		void SetPespective(f32 fov, f32 aspect, f32 znear, f32 zfar);

		// changes current aspect ratio
		void UpdateAspectRatio(f32 aspect);

		// changes current position vector
		void SetPosition(glm::vec3 position);

		// changes current rotation vector
		void SetRotation(glm::vec3 rotation);

		// changes current translation vector
		void SetTranslation(glm::vec3 translation);

		// rotates by a given value
		void Rotate(glm::vec3 delta);

		// translates by a given value
		void Translate(glm::vec3 delta);

	private:

		Specification m_Specification;
	};
}