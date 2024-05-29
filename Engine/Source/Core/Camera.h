#pragma once

#include "Event/Event.h"
#include "Util/Math.h"
#include "Util/Memory.h"

#include <memory>

namespace Cosmos
{
	class Camera
	{
	public:

		enum Type
		{
			LOOK_AT = 0,
			FREE_LOOK
		};

	public:

		// constructor
		Camera();

		// destructor
		~Camera() = default;

	public:

		// enables/disables the camera movement
		inline void SetMove(bool value) { mShouldMove = value; }

		// query if camera movement is enabled
		inline bool CanMove() const { return mShouldMove; }

		// returns camera's mode
		inline Type GetType() const { return mType; }

		// sets a new camera's mode
		inline void SetType(Type type) { mType = type; }

		// sets a new aspect ratio for the camera
		inline void SetAspectRatio(float aspect) { mAspectRatio = aspect; }

		// returns the camera's aspect ratio
		inline float GetAspectRatio() const { return mAspectRatio; }

		// returns the camera field of view
		inline float GetFov() const { return mFov; }

		// returns near value
		inline float GetNear() const { return mZnear; }

		// returns far value
		inline float GetFar() const { return mZfar; }

		// returns the camera eyes
		inline glm::vec3& GetFrontRef() { return mFront; }

		// returns the current camera position
		inline glm::vec3& GetPositionRef() { return mPosition; }

		// returns the current camera rotation
		inline glm::vec3& GetRotationRef() { return mRotation; }

	public:

		// returns a reference to the perspective/projection matrix, using defaults aspect ratio
		glm::mat4& GetProjectionRef();

		// returns a reference to the view matrix
		glm::mat4& GetViewRef();

	public:

		// updates the camera
		void OnUpdate(float timestep);

		// event handling
		void OnEvent(Shared<Event> event);

	private:

		// updates the camera's view
		void UpdateViewMatrix();

		// translates the camera
		void Translate(glm::vec3 delta);

		// rotates the camera
		void Rotate(glm::vec3 delta);

	private:

		Type mType = Type::FREE_LOOK;
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

		glm::vec3 mFront = glm::vec3(0.0f);
		glm::vec3 mRotation = glm::vec3(0.0f);
		glm::vec3 mPosition = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 mScale = glm::vec3(1.0f);

		// W A S D
		bool mMovingForward = false;
		bool mMovingBackward = false;
		bool mMovingLeft = false;
		bool mMovingRight = false;
	};
}