#include "MeshLoader.h"

#include <vector>
#include <iostream>
#include <fstream>

namespace LSIS {


	namespace MeshLoader {

		std::shared_ptr<Mesh> LoadFromOBJ(const std::string& filepath)
		{
			std::vector<glm::vec3> vertices{};
			std::vector<glm::uvec3> faces{};

			std::ifstream file(filepath, std::ios::in);
			if (!file.is_open()) {
				std::cerr << "Failed to open " << filepath << std::endl;
			}

			std::cout << "loading file: " << filepath << std::endl;

			std::string buf;

			// scan for number of vertices and faces
			size_t num_vertices = 0, num_faces = 0;
			while (file >> buf) {
				if (buf == "v") {
					num_vertices++;
					continue;
				}
				if (buf == "f") {
					num_faces++;
					continue;
				}
			}

			// resize vectors for for faster reading
			vertices.resize(num_vertices);
			faces.resize(num_faces);

			// reset file
			file.clear();
			file.seekg(0, std::ios::beg);

			size_t index_v = 0, index_f = 0;

			while (file >> buf) {
				switch (buf[0])
				{
				case 'v':
					if (buf == "v") {
						float x, y, z;
						file >> x >> y >> z;
						vertices[index_v++] = { x,y,z };
					}
					else {
						file.ignore(1024, '\n');
					}
					break;
				case 'f':
					if (buf == "f") {
						unsigned int x, y, z;
						file >> x >> y >> z;
						faces[index_f++] = { x - 1, y - 1, z - 1 };
					}
					else {
						file.ignore(1024, '\n');
					}
				default:
					file.ignore(1024, '\n');
					break;
				}
			}

			return std::make_shared<Mesh>(vertices, faces);
		}

		std::shared_ptr<Mesh> CreateRect(glm::vec2 size)
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

			return std::make_shared<Mesh>(vertices, faces);
		}

		std::shared_ptr<Mesh> CreateCube(float size)
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

			return std::make_shared<Mesh>(vertices, faces);
		}
	}
}