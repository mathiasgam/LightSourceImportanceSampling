#include "GeometryBuffer.h"
#include "glad/glad.h"

namespace LSIS {


	GeometryBuffer::GeometryBuffer(std::shared_ptr<Mesh> mesh)
	{
		m_num_faces = mesh->GetNumFaces();

		glGenBuffers(1, &m_vbo);
		glGenBuffers(1, &m_ebo);
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->GetNumVertices(), mesh->GetVertices(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->GetNumFaces(), mesh->GetFaces(), GL_STATIC_DRAW);
	}

	GeometryBuffer::~GeometryBuffer()
	{
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
	}

	void GeometryBuffer::Bind()
	{
		glBindVertexArray(m_vao);
	}

}