#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {

	Mesh::Mesh(const std::string& path)
	{
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces, glm::vec3 color)
	{
		m_color = color;

		// resize vectors to fit data
		m_vertices.resize(vertices.size() * 3);
		m_faces.resize(faces.size() * 3);

		int index = 0;
		for (auto& vertex : vertices) {
			m_vertices[index++] = vertex.x;
			m_vertices[index++] = vertex.y;
			m_vertices[index++] = vertex.z;
		}

		index = 0;
		for (auto& face : faces) {
			m_faces[index++] = face.x;
			m_faces[index++] = face.y;
			m_faces[index++] = face.z;
		}
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
	}

	void Mesh::Upload()
	{
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_faces.size(), m_faces.data(), GL_STATIC_DRAW);
	}

	void Mesh::Render()
	{
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, m_faces.size(), GL_UNSIGNED_INT, nullptr);
	}

	void Mesh::SetColor(const glm::vec3& color)
	{
		m_color = color;
	}

	const glm::vec3& Mesh::GetColor() const
	{
		return m_color;
	}

	std::shared_ptr<Mesh> Mesh::CreateSquare(glm::vec3 center, glm::vec2 size, glm::vec3 color)
	{
		std::vector<glm::vec3> vertices{};
		std::vector<glm::uvec3> faces{};

		float hw = size.x / 2;
		float hh = size.y / 2;

		vertices.emplace_back(center.x - hw, center.y - hh, center.z);
		vertices.emplace_back(center.x + hw, center.y - hh, center.z);
		vertices.emplace_back(center.x + hw, center.y + hh, center.z);
		vertices.emplace_back(center.x - hw, center.y + hh, center.z);

		faces.emplace_back(0, 1, 2);
		faces.emplace_back(0, 2, 3);

		return std::make_shared<Mesh>(vertices, faces, color);
	}

}