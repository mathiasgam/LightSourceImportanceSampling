#include "pch.h"
#include "Material.h"

namespace LSIS {

	Material::Material(std::shared_ptr<Shader> shader, glm::vec4 color)
		: m_shader(shader), m_diffuse(color), m_specular({0.0f,0.0f,0.0f})
	{
	}

	Material::~Material()
	{
	}

	void Material::Bind(const glm::mat4& transform, const glm::mat4& cam_matrix)
	{
		m_shader->Bind();
		m_shader->UploadUniformfloat3("color", m_diffuse);
		m_shader->UploadUniformMat4("model", transform);
		m_shader->UploadUniformMat4("cam_matrix", cam_matrix);
	}

}