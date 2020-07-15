#include "MeshLoader.h"

#include <vector>
#include <iostream>
#include <fstream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace LSIS {


	namespace MeshLoader {

		std::shared_ptr<MeshData> LoadFromOBJ(const std::string& filepath)
		{

			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes{};
			std::vector<tinyobj::material_t> materials{};

			std::string warn;
			std::string err;

			bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

			std::vector<VertexData> vertices{};
			std::vector<IndexData> faces{};

			const uint32_t num_vertices = attrib.vertices.size() / 3;
			const uint32_t num_faces = shapes[0].mesh.indices.size() / 3;

			vertices.resize(num_vertices);
			faces.resize(num_faces);

			for (int i = 0; i < num_vertices; i++) {
				VertexData v{};
				const uint32_t index = i * 3;
				v.position[0] = attrib.vertices[index];
				v.position[1] = attrib.vertices[index+1];
				v.position[2] = attrib.vertices[index+2];
				v.normal[0] = 0.0f;
				v.normal[1] = 0.0f;
				v.normal[2] = 0.0f;
				v.uv[0] = 0.0f;
				v.uv[1] = 0.0f;
				vertices[i] = v;
			}

			for (int i = 0; i < num_faces; i++) {
				IndexData f{};
				const uint32_t index = i * 3;
				f.x = shapes[0].mesh.indices[index].vertex_index;
				f.y = shapes[0].mesh.indices[index+1].vertex_index;
				f.z = shapes[0].mesh.indices[index+2].vertex_index;
				faces[i] = f;
			}

			return std::make_shared<MeshData>(vertices, faces);
		}

		std::shared_ptr<MeshData> CreateRect(glm::vec2 size)
		{
			std::vector<VertexData> vertices{};
			std::vector<IndexData> faces{};

			float hw = size.x / 2;
			float hh = size.y / 2;

			vertices.emplace_back(-hw, -hh, 0, 0, 0, 1, 0, 0);
			vertices.emplace_back(+hw, -hh, 0, 0, 0, 1, 1, 0);
			vertices.emplace_back(+hw, +hh, 0, 0, 0, 1, 1, 1);
			vertices.emplace_back(-hw, +hh, 0, 0, 0, 1, 0, 1);

			faces.emplace_back(0, 1, 2);
			faces.emplace_back(0, 2, 3);

			return std::make_shared<MeshData>(vertices, faces);
		}

		std::shared_ptr<MeshData> CreateCube(float size)
		{
			std::vector<VertexData> vertices{};
			std::vector<IndexData> faces{};

			float hs = size / 2.0f;

			vertices.emplace_back(hs, hs, -hs, 1, 1, -1, 0, 0);
			vertices.emplace_back(hs, -hs, -hs, 1, -1, -1, 0, 0);
			vertices.emplace_back(hs, hs, hs, 1, 1, 1, 0, 0);
			vertices.emplace_back(hs, -hs, hs, 1, -1, 1, 0, 0);

			vertices.emplace_back(-hs, hs, -hs, -1, 1, -1, 0, 0);
			vertices.emplace_back(-hs, -hs, -hs, -1, -1, -1, 0, 0);
			vertices.emplace_back(-hs, hs, hs, -1, 1, 1, 0, 0);
			vertices.emplace_back(-hs, -hs, hs, -1, -1, 1, 0, 0);

			faces.emplace_back(4, 2, 0);
			faces.emplace_back(2, 7, 3);
			faces.emplace_back(6, 5, 7);
			faces.emplace_back(1, 7, 5);
			faces.emplace_back(0, 3, 1);
			faces.emplace_back(4, 1, 5);
			faces.emplace_back(4, 6, 2);
			faces.emplace_back(2, 6, 7);
			faces.emplace_back(6, 4, 5);
			faces.emplace_back(1, 3, 7);
			faces.emplace_back(0, 2, 3);
			faces.emplace_back(4, 0, 1);

			return std::make_shared<MeshData>(vertices, faces);
		}
	}
}