#pragma once

#include <memory>

#include "glm.hpp"

#include "Graphics/Shader.h"
#include "Transform.h"

namespace LSIS {

	class Material {
	public:
		Material(std::shared_ptr<Shader> shader, glm::vec4 color);
		virtual ~Material();

		void Bind(const glm::mat4& transform, const glm::mat4& cam_matrix);

		glm::vec3 GetDiffuse() const { return m_diffuse; }
		glm::vec3 GetSpecular() const { return m_specular; }

	private:
		std::shared_ptr<Shader> m_shader;
		glm::vec3 m_diffuse;
		glm::vec3 m_specular;
		
	};

}