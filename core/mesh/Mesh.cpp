#include "Mesh.h"

#include "glad/glad.h"

namespace LSIS {

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

	}

	Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& faces)
	{
		m_vertices = vertices;
		m_faces = faces;
	}

	Mesh::~Mesh()
	{
	}

	const float* Mesh::GetVertices() const
	{
		return m_vertices.data();
	}

	const unsigned int* Mesh::GetFaces() const
	{
		return m_faces.data();
	}

	const unsigned int Mesh::GetNumVertices() const
	{
		return m_vertices.size();
	}

	const unsigned int Mesh::GetNumFaces() const
	{
		return m_faces.size();
	}


	

}