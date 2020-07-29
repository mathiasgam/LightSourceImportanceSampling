#include "pch.h"
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

	glm::vec3 Camera::GetPosition() const
	{
		return m_transform.GetPosition();
	}

	glm::vec3 Camera::GetRotation() const
	{
		return m_transform.GetRotation();
	}

	glm::mat4 Camera::GetModelMatrix() const
	{
		return m_transform.GetModelMatrix();
	}

	glm::mat4 Camera::GetViewProjectionMatrix() const
	{
		float aspect = static_cast<float>(m_resolution.x) / static_cast<float>(m_resolution.y);
		glm::mat4 P = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 100.0f);
		glm::mat4 V = m_transform.GetModelMatrix();
		return P * glm::inverse(V);
	}

}