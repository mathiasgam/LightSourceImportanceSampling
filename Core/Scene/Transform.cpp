#include "pch.h"
#include "Transform.h"

#include "gtx/transform.hpp"

namespace LSIS {
	Transform::Transform()
		: m_position(0, 0, 0), m_rotation(0, 0, 0), m_model_matrix()
	{
	}

	Transform::Transform(glm::vec3 position, glm::vec3 rotation)
		: m_position(position), m_rotation(rotation), m_model_matrix()
	{
	}

	Transform::~Transform()
	{
	}

	void Transform::SetPosition(glm::vec3 position)
	{
		m_position = position;
	}

	void Transform::SetRotation(glm::vec3 rotation)
	{
		m_rotation = rotation;
	}

	glm::mat4 Transform::GetModelMatrix() const
	{
		glm::mat4 T = glm::mat4(1.0f);
		T *= glm::translate(m_position);
		T *= glm::rotate(m_rotation.z, glm::vec3(0, 0, 1));
		T *= glm::rotate(m_rotation.y, glm::vec3(0, 1, 0));
		T *= glm::rotate(m_rotation.x, glm::vec3(1, 0, 0));
		return T;
	}
}