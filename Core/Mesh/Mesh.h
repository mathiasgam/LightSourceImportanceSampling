#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {

	struct MaterialData {
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 emissive;
	};

	struct FaceData {
		int vertex0;
		int vertex1;
		int vertex2;
		int material;
	};

	struct VertexData {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;

		VertexData()
			: position( 0.0f,0.0f,0.0f ), normal( 0.0f,0.0f,0.0f ), uv( 0.0f,0.0f )
		{
		}

		VertexData(glm::vec3 position, glm::vec3 normal, glm::vec2 uv) 
			: position(position), normal(normal), uv(uv)
		{
		}

		VertexData(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: position( px,py,pz ), normal( nx,ny,nz ), uv( u,v )
		{
		}
	};

	class MeshData {
	public:
		MeshData(const std::vector<VertexData>& vertices, const std::vector<FaceData>& faces, const std::vector<MaterialData>& materials);
		virtual ~MeshData();

		const VertexData* GetVertices() const { return m_vertices.data(); }
		const FaceData* GetIndices() const { return m_indices.data(); }
		const MaterialData* GetMaterials() const { return m_materials.data(); }
		size_t GetNumVertices() const { return m_vertices.size(); }
		size_t GetNumIndices() const { return m_indices.size(); }
		size_t GetNumMaterials() const { return m_materials.size(); }

	private:
		std::vector<VertexData> m_vertices;
		//std::vector<float> m_vertices;
		//std::vector<float> m_normals;
		std::vector<FaceData> m_indices;
		std::vector<MaterialData> m_materials;
	};

	class Mesh {
	public:
		Mesh(std::shared_ptr<MeshData> data);
		virtual ~Mesh();

		void Upload(std::shared_ptr<MeshData> data);
		void Bind();

		inline unsigned int GetNumIndices() const { return m_num_indices; }
		inline unsigned int GetNumVertices() const { return m_num_vertices; };

		std::shared_ptr<MeshData> GetData() const;

	private:
		unsigned int m_num_indices;
		unsigned int m_num_vertices;
		unsigned int m_vbo;
		unsigned int m_ebo;
		unsigned int m_vao;

		std::shared_ptr<MeshData> m_data;
	};

}