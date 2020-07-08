#include "Camera.h"

#include "gtx/transform.hpp"

namespace LSIS {

	Camera::Camera(glm::vec3 position, glm::vec3 rotation, glm::uvec2 resolution)
		: m_transform(position, rotation), m_resolution(resolution)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(glm::vec3 position)
	{
		m_transform.SetPosition(position);
	}

	void Camera::SetRotation(glm::vec3 rotation)
	{
		m_transform.SetRotation(rotation);
	}

	void Camera::SetResolution(glm::uvec2 resolution)
	{
		m_resolution = resolution;
	}

	void Camera::SetFOV(float fov)
	{
		m_FOV = fov;
	}

	void Camera::LookAt(glm::vec3 position)
	{
		//m_direction = glm::normalize(position - m_position);
	}

	glm::mat4 Camera::GetViewProjectionMatrix() const
	{
		glm::mat4 P = glm::perspectiveFov(m_FOV, 1.0f, 1.0f, 0.1f, 100.0f);
		glm::mat4 V = m_transform.GetModelMatrix();
		return glm::inverse(V * P);
	}

}