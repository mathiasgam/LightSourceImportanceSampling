#pragma once

#include "glm.hpp"

namespace LSIS {

	class Light
	{
	public:
		Light(glm::vec3 position, glm::vec3 color);
		virtual ~Light();

	private:
		glm::vec4 m_position;
		glm::vec4 m_color;
		glm::vec4 m_extra;
	};

}
