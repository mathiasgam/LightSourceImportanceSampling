#include "pch.h"
#include "Material.h"

namespace LSIS {

	Material::Material(std::shared_ptr<Shader> shader, glm::vec4 color)
		: m_shader(shader), m_color(color)
	{
	}

	Material::~Material()
	{
	}

	void Material::Bind(const glm::mat4& transform, const glm::mat4& cam_matrix)
	{
		m_shader->Bind();
		m_shader->UploadUniformfloat4("color", m_color);
		m_shader->UploadUniformMat4("model", transform);
		m_shader->UploadUniformMat4("cam_matrix", cam_matrix);
	}

}