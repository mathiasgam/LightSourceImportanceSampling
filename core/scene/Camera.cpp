#include "Camera.h"

namespace LSIS {

	Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::uvec2 resolution)
		: m_position(position), m_direction(direction), m_resolution(resolution)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(glm::vec3 position)
	{
		m_position = position;
	}

	void Camera::SetDirection(glm::vec3 direction)
	{
		m_direction = direction;
	}

	void Camera::SetResolution(glm::uvec2 resolution)
	{
		m_resolution = resolution;
	}

	void Camera::LookAt(glm::vec3 position)
	{
		m_direction = glm::normalize(position - m_position);
	}

}