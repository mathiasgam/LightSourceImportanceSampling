#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {

	Mesh::Mesh(const std::string& path)
	{
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces)
	{
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

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
	}

	void Mesh::Render()
	{
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, m_faces.size(), GL_UNSIGNED_INT, nullptr);
		//glDrawElements(GL_LINE_LOOP, m_faces.size(), GL_UNSIGNED_INT, nullptr);
	}

	std::shared_ptr<Mesh> Mesh::CreateRect(glm::vec2 size)
	{
		std::vector<glm::vec3> vertices{};
		std::vector<glm::uvec3> faces{};

		float hw = size.x / 2;
		float hh = size.y / 2;

		vertices.emplace_back(-hw, -hh, 0);
		vertices.emplace_back(+hw, -hh, 0);
		vertices.emplace_back(+hw, +hh, 0);
		vertices.emplace_back(-hw, +hh, 0);

		faces.emplace_back(0, 1, 2);
		faces.emplace_back(0, 2, 3);

		return std::make_shared<Mesh>(vertices, faces);
	}

	std::shared_ptr<Mesh> Mesh::CreateCube(float size)
	{
		std::vector<glm::vec3> vertices{};
		std::vector<glm::uvec3> faces{};

		float hs = size / 2.0f;

		vertices.emplace_back(hs, hs, -hs);
		vertices.emplace_back(hs, -hs, -hs);
		vertices.emplace_back(hs, hs, hs);
		vertices.emplace_back(hs, -hs, hs);

		vertices.emplace_back(-hs, hs, -hs);
		vertices.emplace_back(-hs, -hs, -hs);
		vertices.emplace_back(-hs, hs, hs);
		vertices.emplace_back(-hs, -hs, hs);

		faces.emplace_back(4, 2, 0);
		faces.emplace_back(2, 7, 3);
		faces.emplace_back(6, 5, 7);
		faces.emplace_back(1, 7, 5);
		faces.emplace_back(0, 3, 1);
		faces.emplace_back(4, 1, 5);
		faces.emplace_back(4, 6, 2);
		faces.emplace_back(2, 6, 7);
		faces.emplace_back(6, 4, 5);
		faces.emplace_back(1, 3, 7);
		faces.emplace_back(0, 2, 3);
		faces.emplace_back(4, 0, 1);

		return std::make_shared<Mesh>(vertices, faces);
	}

}