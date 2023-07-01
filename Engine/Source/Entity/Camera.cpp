#include "Camera.h"

namespace Cosmos
{
	Camera::Camera()
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rot = glm::mat4(1.0f);
		rot = glm::rotate(rot, glm::radians(m_Specification.Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rot = glm::rotate(rot, glm::radians(m_Specification.Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rot = glm::rotate(rot, glm::radians(m_Specification.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Specification.Position * glm::vec3(1.0f, 1.0f, -1.0f));

		if (m_Specification.Type == Type::FIRST_PERSON)
		{
			m_Specification.View = rot * transform;
		}

		else
		{
			m_Specification.View = transform * rot;
		}

		m_Specification.Updated = true;
	}

	bool Camera::IsMoving()
	{
		return Input::Get().IsKeyDown(Input::Keycode::KEYCODE_W) || Input::Get().IsKeyDown(Input::Keycode::KEYCODE_S)
			|| Input::Get().IsKeyDown(Input::Keycode::KEYCODE_A) || Input::Get().IsKeyDown(Input::Keycode::KEYCODE_D);
	}

	void Camera::SetPespective(f32 fov, f32 aspect, f32 znear, f32 zfar)
	{
		m_Specification.FieldOfView = fov;
		m_Specification.ZNearClip = znear;
		m_Specification.ZFarClip = zfar;
		m_Specification.Perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}

	void Camera::Update(f32 deltaTime)
	{
		m_Specification.Updated = false;

		if (m_Specification.Type == Camera::Type::FIRST_PERSON && IsMoving())
		{
			glm::vec3 front;
			front.x = -cos(glm::radians(m_Specification.Rotation.x)) * sin(glm::radians(m_Specification.Rotation.y));
			front.y = sin(glm::radians(m_Specification.Rotation.x));
			front.z = cos(glm::radians(m_Specification.Rotation.x)) * cos(glm::radians(m_Specification.Rotation.y));

			f32 speed = deltaTime * m_Specification.MovementSpeed;

			if (Input::Get().IsKeyDown(Input::Keycode::KEYCODE_W)) m_Specification.Position += front * speed;
			if (Input::Get().IsKeyDown(Input::Keycode::KEYCODE_S)) m_Specification.Position -= front * speed;
			if (Input::Get().IsKeyDown(Input::Keycode::KEYCODE_A)) m_Specification.Position -= glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * speed;
			if (Input::Get().IsKeyDown(Input::Keycode::KEYCODE_D)) m_Specification.Position += glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * speed;

			UpdateViewMatrix();
		}
	}

	void Camera::UpdateAspectRatio(f32 aspect)
	{
		m_Specification.Perspective = glm::perspective(glm::radians(m_Specification.FieldOfView), aspect, m_Specification.ZNearClip, m_Specification.ZFarClip);
	}

	void Camera::SetPosition(glm::vec3 position)
	{
		m_Specification.Position = position;
		UpdateViewMatrix();
	}

	void Camera::SetRotation(glm::vec3 rotation)
	{
		m_Specification.Rotation = rotation;
		UpdateViewMatrix();
	}

	void Camera::SetTranslation(glm::vec3 translation)
	{
		m_Specification.Position = translation;
		UpdateViewMatrix();
	}

	void Camera::Rotate(glm::vec3 delta)
	{
		m_Specification.Rotation += delta;
		UpdateViewMatrix();
	}

	void Camera::Translate(glm::vec3 delta)
	{
		m_Specification.Position += delta;
		UpdateViewMatrix();
	}
}