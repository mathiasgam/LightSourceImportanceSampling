#include "pch.h"
#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {	

	MeshData::MeshData(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices) {
		m_vertices.resize(vertices.size());
		m_indices.resize(indices.size());
		std::copy(vertices.begin(), vertices.end(), m_vertices.begin());
		std::copy(indices.begin(), indices.end(), m_indices.begin());
	}

	MeshData::~MeshData()
	{
	}

	Mesh::Mesh(std::shared_ptr<MeshData> data)
		: m_num_indices(0), m_num_vertices(0), m_vbo(0), m_ebo(0), m_vao(0)
	{
		Upload(data);
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
		m_num_indices = 0;
		m_num_vertices = 0;
	}

	void Mesh::Upload(std::shared_ptr<MeshData> data)
	{
		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);
		glGenVertexArrays(1, &m_vao);

		m_num_vertices = (unsigned int) data->GetNumVertices();
		m_num_indices = (unsigned int) data->GetNumIndices();

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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * data->GetNumIndices(), data->GetIndices(), GL_STATIC_DRAW);
	}

	void Mesh::Bind()
	{
		glBindVertexArray(m_vao);
	}

	std::shared_ptr<MeshData> Mesh::Download() const
	{
		std::vector<VertexData> vertices{};
		std::vector<uint32_t> indices{};

		vertices.resize(m_num_vertices);
		indices.resize(m_num_indices);

		glGetBufferSubData(m_vbo, 0, m_num_vertices, vertices.data());
		glGetBufferSubData(m_ebo, 0, m_num_indices, indices.data());

		return std::make_shared<MeshData>(vertices, indices);
	}

}