#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {	

	MeshData::MeshData(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces)
	{
		// resize vectors to fit data
		m_vertices.resize(vertices.size());
		m_faces.resize(faces.size());

		int index = 0;
		for (auto& vertex : vertices) {
			VertexData v{};
			v.position[0] = vertex.x;
			v.position[1] = vertex.y;
			v.position[2] = vertex.z;
			v.normal[0] = 0.0f;
			v.normal[1] = 0.0f;
			v.normal[2] = 0.0f;
			v.uv[0] = 0.0f;
			v.uv[1] = 0.0f;
			m_vertices[index++] = v;
		}

		index = 0;
		for (auto& face : faces) {
			IndexData i{};
			i.x = face.x;
			i.y = face.y;
			i.z = face.z;
			m_faces[index++] = i;
		}
	}

	MeshData::MeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& faces)
	{
		const uint32_t num_vertices = vertices.size() / 3;
		const uint32_t num_faces = faces.size() / 3;

		// resize vectors to fit data
		m_vertices.resize(vertices.size() / 3);
		m_faces.resize(faces.size() / 3);


		for (uint32_t i = 0; i < num_vertices; i++) {
			VertexData v{};
			int index = i * 3;
			v.position[0] = vertices[index];
			v.position[1] = vertices[index+1];
			v.position[2] = vertices[index+2];
			v.normal[0] = 0.0f;
			v.normal[1] = 0.0f;
			v.normal[2] = 0.0f;
			v.uv[0] = 0.0f;
			v.uv[1] = 0.0f;
			m_vertices[i] = v;
		}

		for (uint32_t i = 0; i < num_faces; i++) {
			IndexData f{};
			uint32_t index = i * 3;
			f.x = faces[index];
			f.y = faces[index+1];
			f.z = faces[index+2];
			m_faces[i] = f;
		}
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
		m_num_faces = (unsigned int) data->GetNumFaces() * 3;

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * data->GetNumVertices(), data->GetVertices(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, uv));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexData) * data->GetNumFaces(), data->GetFaces(), GL_STATIC_DRAW);
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