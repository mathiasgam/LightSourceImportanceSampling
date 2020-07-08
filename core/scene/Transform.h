#pragma once

#include "glm.hpp"

namespace LSIS {


	class Transform
	{
	public:
		Transform();
		Transform(glm::vec3 position, glm::vec3 rotation = { 0,0,0 });
		virtual ~Transform();

		void SetPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotation);

		glm::mat4 GetModelMatrix() const;

	private:
		glm::vec3 m_position;
		glm::vec3 m_rotation;

		glm::mat4 m_model_matrix;
	};


}