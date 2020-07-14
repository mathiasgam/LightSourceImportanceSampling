#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {	

	MeshData::MeshData(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces)
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
	}

	MeshData::MeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& faces)
	{
		m_vertices = vertices;
		m_faces = faces;
	}

	MeshData::~MeshData()
	{
	}

	Mesh::Mesh()
		: m_num_faces(0), m_vbo(0), m_ebo(0), m_vao(0)
	{
	}

	Mesh::Mesh(std::shared_ptr<MeshData> data)
		: m_num_faces(0), m_vbo(0), m_ebo(0), m_vao(0)
	{
		Upload(data);
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
		m_num_faces = 0;
	}

	void Mesh::Upload(std::shared_ptr<MeshData> data)
	{
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);
		glGenVertexArrays(1, &m_vao);

		m_num_vertices = (unsigned int) data->GetNumVertices();
		m_num_faces = (unsigned int) data->GetNumFaces();

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data->GetNumVertices(), data->GetVertices(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * data->GetNumFaces(), data->GetFaces(), GL_STATIC_DRAW);
	}

	void Mesh::Bind()
	{
		glBindVertexArray(m_vao);
	}

	std::shared_ptr<MeshData> Mesh::Download() const
	{
		std::vector<float> vertices{};
		std::vector<unsigned int> faces{};

		vertices.resize(m_num_vertices);
		faces.resize(m_num_faces);

		glGetBufferSubData(m_vbo, 0, m_num_vertices, vertices.data());
		glGetBufferSubData(m_ebo, 0, m_num_faces, faces.data());

		return std::make_shared<MeshData>(vertices, faces);
	}

}