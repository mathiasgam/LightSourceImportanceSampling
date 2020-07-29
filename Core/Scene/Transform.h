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
		inline glm::vec3 GetPosition() const { return m_position; }
		inline glm::vec3 GetRotation() const { return m_rotation; }

	private:
		glm::vec3 m_position;
		glm::vec3 m_rotation;

		glm::mat4 m_model_matrix;
	};


}