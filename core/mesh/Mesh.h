#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {


	class Mesh {
	public:
		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces);
		virtual ~Mesh();

		void Render();

	private:
		std::vector<float> m_vertices;
		std::vector<unsigned int> m_faces;

		unsigned int m_ebo = 0;
		unsigned int m_vbo = 0;
		unsigned int m_vao = 0;
	};

}