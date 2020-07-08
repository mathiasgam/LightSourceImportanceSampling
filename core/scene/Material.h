#pragma once

#include <memory>

#include "glm.hpp"
#include "graphics/Shader.h"
#include "Transform.h"

namespace LSIS {

	class Material {
	public:
		Material(std::shared_ptr<Shader> shader, glm::vec4 color);
		virtual ~Material();

		void Bind(const Transform& transform, const glm::mat4& cam_matrix);

	private:
		std::shared_ptr<Shader> m_shader;
		glm::vec4 m_color;
		
	};

}