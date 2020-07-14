#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {


	class Mesh {
	public:
		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces);
		Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& faces);
		virtual ~Mesh();

		const float* GetVertices() const;
		const unsigned int* GetFaces() const;
		const unsigned int GetNumVertices() const;
		const unsigned int GetNumFaces() const;

	private:
		std::vector<float> m_vertices{};
		std::vector<unsigned int> m_faces{};


	};

}