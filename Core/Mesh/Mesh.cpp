#include "pch.h"
#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {	

	MeshData::MeshData(const std::vector<VertexData>& vertices, const std::vector<FaceData>& indices, const std::vector<MaterialData>& materials) {
		m_vertices.resize(vertices.size());
		m_indices.resize(indices.size());
		std::copy(vertices.begin(), vertices.end(), m_vertices.begin());
		std::copy(indices.begin(), indices.end(), m_indices.begin());
		if (materials.size() > 0) {
			m_materials.resize(materials.size());
			std::copy(materials.begin(), materials.end(), m_materials.begin());
		}
		else {
			m_materials.resize(1);
			MaterialData mat = {};
			mat.diffuse = { 0.8f, 0.8f, 0.8f };
			mat.specular = { 0.8f, 0.8f, 0.8f };
			mat.emissive = { 0.0f, 0.0f, 0.0f };
			m_materials[0] = mat;
		}
	}

	MeshData::~MeshData()
	{
	}

	Mesh::Mesh(std::shared_ptr<MeshData> data)
		: m_num_indices(0), m_num_vertices(0), m_vbo(0), m_ebo(0), m_vao(0)
	{
		Upload(data);
		m_data = data;
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

		const size_t num_faces = data->GetNumIndices();
		m_num_vertices = (unsigned int) data->GetNumVertices();
		m_num_indices = (unsigned int) num_faces * 3;

		auto faces = data->GetIndices();

		unsigned int* indices = new unsigned int[m_num_indices];
		for (int i = 0; i < num_faces; i++) {
			FaceData face = faces[i];
			indices[3 * i + 0] = face.vertex0;
			indices[3 * i + 1] = face.vertex1;
			indices[3 * i + 2] = face.vertex2;
		}

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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_num_indices, indices, GL_STATIC_DRAW);

		delete[] indices;
	}

	void Mesh::Bind()
	{
		glBindVertexArray(m_vao);
	}

	std::shared_ptr<MeshData> Mesh::GetData() const
	{
		return m_data;
	}

}