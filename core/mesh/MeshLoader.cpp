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

			std::vector<float> vertices{};
			std::vector<unsigned int> faces{};

			vertices.resize(attrib.vertices.size());
			faces.resize(shapes[0].mesh.indices.size());

			for (int i = 0; i < vertices.size(); i++) {
				vertices[i] = attrib.vertices[i];
			}

			for (int i = 0; i < faces.size(); i++) {
				faces[i] = shapes[0].mesh.indices[i].vertex_index;
			}

			return std::make_shared<MeshData>(vertices, faces);
		}

		std::shared_ptr<MeshData> CreateRect(glm::vec2 size)
		{
			std::vector<glm::vec3> vertices{};
			std::vector<glm::uvec3> faces{};

			float hw = size.x / 2;
			float hh = size.y / 2;

			vertices.emplace_back(-hw, -hh, 0);
			vertices.emplace_back(+hw, -hh, 0);
			vertices.emplace_back(+hw, +hh, 0);
			vertices.emplace_back(-hw, +hh, 0);

			faces.emplace_back(0, 1, 2);
			faces.emplace_back(0, 2, 3);

			return std::make_shared<MeshData>(vertices, faces);
		}

		std::shared_ptr<MeshData> CreateCube(float size)
		{
			std::vector<glm::vec3> vertices{};
			std::vector<glm::uvec3> faces{};

			float hs = size / 2.0f;

			vertices.emplace_back(hs, hs, -hs);
			vertices.emplace_back(hs, -hs, -hs);
			vertices.emplace_back(hs, hs, hs);
			vertices.emplace_back(hs, -hs, hs);

			vertices.emplace_back(-hs, hs, -hs);
			vertices.emplace_back(-hs, -hs, -hs);
			vertices.emplace_back(-hs, hs, hs);
			vertices.emplace_back(-hs, -hs, hs);

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