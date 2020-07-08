#include "Transform.h"

#include "gtx/transform.hpp"

namespace LSIS {
	Transform::Transform()
		: m_position(0,0,0), m_rotation(0,0,0), m_model_matrix()
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
		return glm::translate(m_position);
	}
}