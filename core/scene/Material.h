#pragma once

#include <memory>

#include "glm.hpp"
#include "graphics/Shader.h"

namespace LSIS {

	class Material {
	public:


	private:
		glm::vec4 color;
		std::shared_ptr<Shader> m_shader;
		
	};

}