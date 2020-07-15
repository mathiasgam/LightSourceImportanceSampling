#pragma once

#include "glm.hpp"

namespace LSIS {

	class Light
	{
	public:
		Light(glm::vec3 position, glm::vec3 color);
		virtual ~Light();

		inline glm::vec3 GetPosition() const { return m_position; }
		inline glm::vec3 GetColor() const { return m_color; }

	private:
		glm::vec3 m_position;
		glm::vec3 m_color;
		glm::vec2 m_extra;
	};

}
