#include "pch.h"
#include "MeshLoader.h"

#include <vector>
#include <iostream>
#include <unordered_map>

#include "gtx/hash.hpp"

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
			std::vector<uint32_t> indices{};

			const uint32_t num_vertices = static_cast<uint32_t>(attrib.vertices.size() / 3);
			const uint32_t num_faces = static_cast<uint32_t>(shapes[0].mesh.indices.size() / 3);

			std::unordered_map<glm::uvec3, uint32_t> vertex_map{};

			for (auto& face : shapes[0].mesh.indices) {
				glm::uvec3 key = { face.vertex_index, face.normal_index, face.texcoord_index };
				auto p = vertex_map.find(key);
				if (p == vertex_map.end()) {
					// unique vertex not yet mapped
					uint32_t index = static_cast<uint32_t>(vertices.size());

					VertexData v{};

					v.position[0] = attrib.vertices[3 * face.vertex_index + 0];
					v.position[1] = attrib.vertices[3 * face.vertex_index + 1];
					v.position[2] = attrib.vertices[3 * face.vertex_index + 2];
					v.normal[0] = attrib.normals[3 * face.normal_index + 0];
					v.normal[1] = attrib.normals[3 * face.normal_index + 1];
					v.normal[2] = attrib.normals[3 * face.normal_index + 2];
					v.uv[0] = attrib.texcoords[2 * face.texcoord_index];
					v.uv[1] = attrib.texcoords[2 * face.texcoord_index + 1];

					vertices.push_back(v);
					indices.push_back(index);
					vertex_map.insert({ key, index });
				}
				else {
					uint32_t index = p->second;
					indices.push_back(index);
				}
			}



			return std::make_shared<MeshData>(vertices, indices);
		}

		void push_face(std::vector<uint32_t>& vec, glm::uvec3 face) {
			vec.push_back(face.x);
			vec.push_back(face.y);
			vec.push_back(face.z);
		}

		std::shared_ptr<MeshData> CreateRect(glm::vec2 size)
		{
			std::vector<VertexData> vertices{};
			std::vector<uint32_t> indices{};

			float hw = size.x / 2;
			float hh = size.y / 2;

			vertices.emplace_back(-hw, -hh, 0.0f, 00.f, 0.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(+hw, -hh, 0.0f, 00.f, 0.0f, 1.0f, 1.0f, 0.0f);
			vertices.emplace_back(+hw, +hh, 0.0f, 00.f, 0.0f, 1.0f, 1.0f, 1.0f);
			vertices.emplace_back(-hw, +hh, 0.0f, 00.f, 0.0f, 1.0f, 0.0f, 1.0f);

			push_face(indices, { 0,1,2 });
			push_face(indices, { 0,2,3 });

			return std::make_shared<MeshData>(vertices, indices);
		}

		std::shared_ptr<MeshData> CreateCube(float size)
		{
			std::vector<VertexData> vertices{};
			std::vector<uint32_t> indices{};

			float hs = size / 2.0f;

			vertices.emplace_back(hs, hs, -hs, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, -hs, -hs, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, hs, hs, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, -hs, hs, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f);

			vertices.emplace_back(-hs, hs, -hs, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, -hs, -hs, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, hs, hs, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, -hs, hs, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f);


			push_face(indices, { 4, 2, 0 });
			push_face(indices, { 2, 7, 3 });
			push_face(indices, { 6, 5, 7 });
			push_face(indices, { 1, 7, 5 });
			push_face(indices, { 0, 3, 1 });
			push_face(indices, { 4, 1, 5 });
			push_face(indices, { 4, 6, 2 });
			push_face(indices, { 2, 6, 7 });
			push_face(indices, { 6, 4, 5 });
			push_face(indices, { 1, 3, 7 });
			push_face(indices, { 0, 2, 3 });
			push_face(indices, { 4, 0, 1 });

			return std::make_shared<MeshData>(vertices, indices);
		}
	}
}