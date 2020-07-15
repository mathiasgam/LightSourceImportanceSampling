#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {

	struct VertexData {
		float position[3];
		float normal[3];
		float uv[2];

		VertexData()
			: position{ 0.0f,0.0f,0.0f }, normal{ 0.0f,0.0f,0.0f }, uv{ 0.0f,0.0f }
		{
		}

		VertexData(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: position{ px,py,pz }, normal{ nx,ny,nz }, uv{ u,v }
		{
		}
	};

	class MeshData {
	public:
		MeshData(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices);
		virtual ~MeshData();

		const VertexData* GetVertices() const { return m_vertices.data(); }
		const uint32_t* GetIndices() const { return m_indices.data(); }
		size_t GetNumVertices() const { return m_vertices.size(); }
		size_t GetNumIndices() const { return m_indices.size(); }

	private:
		std::vector<VertexData> m_vertices;
		//std::vector<float> m_vertices;
		//std::vector<float> m_normals;
		std::vector<uint32_t> m_indices;
	};

	class Mesh {
	public:
		Mesh(std::shared_ptr<MeshData> data);
		virtual ~Mesh();

		void Upload(std::shared_ptr<MeshData> data);
		void Bind();

		inline unsigned int GetNumIndices() const { return m_num_indices; }
		inline unsigned int GetNumVertices() const { return m_num_vertices; };

		std::shared_ptr<MeshData> Download() const;

	private:
		unsigned int m_num_indices;
		unsigned int m_num_vertices;
		unsigned int m_vbo;
		unsigned int m_ebo;
		unsigned int m_vao;
	};

}