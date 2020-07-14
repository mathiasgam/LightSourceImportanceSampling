#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {


	class MeshData {
	public:
		MeshData(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces);
		MeshData(const std::vector<float>& vertices, const std::vector<unsigned int>& faces);
		virtual ~MeshData();

		const float* GetVertices() const { return m_vertices.data(); }
		const unsigned int* GetFaces() const { return m_faces.data(); }
		size_t GetNumVertices() const { return m_vertices.size(); }
		size_t GetNumFaces() const { return m_faces.size(); }

	private:
		std::vector<float> m_vertices;
		std::vector<unsigned int> m_faces;
	};

	class Mesh {
	public:
		Mesh();
		Mesh(std::shared_ptr<MeshData> data);
		virtual ~Mesh();

		void Upload(std::shared_ptr<MeshData> data);
		void Bind();

		inline unsigned int GetNumFaces() const { return m_num_faces; };
		inline unsigned int GetNumVertices() const { return m_num_vertices; };

		std::shared_ptr<MeshData> Download() const;

	private:
		unsigned int m_num_faces;
		unsigned int m_num_vertices;
		unsigned int m_vbo;
		unsigned int m_ebo;
		unsigned int m_vao;
	};

}