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
			std::vector<tinyobj::shape_t> t_shapes{};
			std::vector<tinyobj::material_t> t_materials{};

			std::string warn;
			std::string err;

			bool ret = tinyobj::LoadObj(&attrib, &t_shapes, &t_materials, &warn, &err, filepath.c_str(), "../Assets/Models/");

			std::vector<uint32_t> indices{};
			std::vector<VertexData> vertices{};
			std::vector<FaceData> faces{};
			std::vector<MaterialData> materials{};

			materials.resize(t_materials.size());
			for (size_t i = 0; i < t_materials.size(); i++) {
				auto& mat = t_materials[i];
				MaterialData data = {};
				data.diffuse = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2] };
				data.specular = { mat.specular[0], mat.specular[1], mat.specular[2] };
				data.emissive = { mat.emission[0], mat.emission[1], mat.emission[2] };

				printf("Material: [%f,%f,%f]\n", data.diffuse.x, data.diffuse.y, data.diffuse.z);
				materials[i] = data;
			}

			const uint32_t num_vertices = static_cast<uint32_t>(attrib.vertices.size() / 3);
			const uint32_t num_faces = static_cast<uint32_t>(t_shapes[0].mesh.indices.size() / 3);

			std::unordered_map<glm::uvec3, uint32_t> vertex_map{};

			for (auto& face : t_shapes[0].mesh.indices) {
				glm::uvec3 key = { face.vertex_index, face.normal_index, face.texcoord_index };
				auto p = vertex_map.find(key);
				if (p == vertex_map.end()) {
					// unique vertex not yet mapped
					uint32_t index = static_cast<uint32_t>(vertices.size());

					VertexData v{};

					v.position.x = attrib.vertices[3 * face.vertex_index + 0];
					v.position.y = attrib.vertices[3 * face.vertex_index + 1];
					v.position.z = attrib.vertices[3 * face.vertex_index + 2];
					v.normal.x = attrib.normals[3 * face.normal_index + 0];
					v.normal.y = attrib.normals[3 * face.normal_index + 1];
					v.normal.z = attrib.normals[3 * face.normal_index + 2];
					v.uv.x = attrib.texcoords[2 * face.texcoord_index];
					v.uv.y = attrib.texcoords[2 * face.texcoord_index + 1];

					vertices.push_back(v);
					indices.push_back(index);
					vertex_map.insert({ key, index });
				}
				else {
					uint32_t index = p->second;
					indices.push_back(index);
				}
			}

			for (size_t i = 0; i < num_faces; i++) {
				FaceData face = {};
				face.vertex0 = indices[3*i];
				face.vertex1 = indices[3*i+1];
				face.vertex2 = indices[3*i+2];
				face.material = t_shapes[0].mesh.material_ids[i];
				faces.push_back(face);
			}

			return std::make_shared<MeshData>(vertices, faces, materials);
		}

		void push_face(std::vector<uint32_t>& vec, glm::uvec3 face) {
			vec.push_back(face.x);
			vec.push_back(face.y);
			vec.push_back(face.z);
		}

		std::shared_ptr<MeshData> CreateRect(glm::vec2 size)
		{
			std::vector<VertexData> vertices{};
			std::vector<FaceData> indices{};
			std::vector<MaterialData> materials{};

			float hw = size.x / 2;
			float hh = size.y / 2;

			vertices.emplace_back(-hw, -hh, 0.0f, 00.f, 0.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(+hw, -hh, 0.0f, 00.f, 0.0f, 1.0f, 1.0f, 0.0f);
			vertices.emplace_back(+hw, +hh, 0.0f, 00.f, 0.0f, 1.0f, 1.0f, 1.0f);
			vertices.emplace_back(-hw, +hh, 0.0f, 00.f, 0.0f, 1.0f, 0.0f, 1.0f);

			indices.push_back({ 0,1,2,0 });
			indices.push_back({ 0,2,3,0 });

			return std::make_shared<MeshData>(vertices, indices, materials);
		}

		std::shared_ptr<MeshData> CreateCube(float size)
		{
			std::vector<VertexData> vertices{};
			std::vector<FaceData> indices{};
			std::vector<MaterialData> materials{};

			float hs = size / 2.0f;

			vertices.emplace_back(hs, hs, -hs, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, -hs, -hs, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, hs, hs, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(hs, -hs, hs, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f);

			vertices.emplace_back(-hs, hs, -hs, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, -hs, -hs, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, hs, hs, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			vertices.emplace_back(-hs, -hs, hs, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f);


			indices.push_back({ 4, 2, 0, 0 });
			indices.push_back({ 2, 7, 3, 0 });
			indices.push_back({ 6, 5, 7, 0 });
			indices.push_back({ 1, 7, 5, 0 });
			indices.push_back({ 0, 3, 1, 0 });
			indices.push_back({ 4, 1, 5, 0 });
			indices.push_back({ 4, 6, 2, 0 });
			indices.push_back({ 2, 6, 7, 0 });
			indices.push_back({ 6, 4, 5, 0 });
			indices.push_back({ 1, 3, 7, 0 });
			indices.push_back({ 0, 2, 3, 0 });
			indices.push_back({ 4, 0, 1, 0 });

			return std::make_shared<MeshData>(vertices, indices, materials);
		}
	}
}