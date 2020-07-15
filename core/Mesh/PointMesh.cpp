#include "PointMesh.h"
#include "glad/glad.h"

namespace LSIS {

	PointMesh::PointMesh(const std::vector<Point>& points)
		: m_num_vertices(0), m_vbo(0), m_vao(0)
	{
		Upload(points);
	}

	PointMesh::~PointMesh()
	{
		m_num_vertices = 0;
		glDeleteBuffers(1, &m_vbo);
		glDeleteVertexArrays(1, &m_vao);
	}

	void PointMesh::Upload(const std::vector<Point>& data)
	{
		m_num_vertices = data.size();

		glGenBuffers(1, &m_vbo);
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * data.size(), data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, color));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	void PointMesh::Bind()
	{
		glBindVertexArray(m_vao);
	}

	std::vector<Point> PointMesh::Download() const
	{
		std::vector<Point> vertices{};

		vertices.resize(m_num_vertices);

		glGetBufferSubData(m_vbo, 0, m_num_vertices, vertices.data());

		return vertices;
	}

}