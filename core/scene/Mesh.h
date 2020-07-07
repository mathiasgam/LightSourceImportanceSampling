#pragma once

#include <vector>
#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {


	class Mesh {
	public:
		Mesh(const std::string& path);
		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> faces, glm::vec3 color = { 1,1,1 });
		virtual ~Mesh();

		void Upload();
		void Render();
		
		void SetColor(const glm::vec3& color);
		const glm::vec3& GetColor() const;

		static std::shared_ptr<Mesh> CreateSquare(glm::vec3 center, glm::vec2 size, glm::vec3 color = { 1,1,1 });

	private:
		std::vector<float> m_vertices;
		std::vector<unsigned int> m_faces;

		unsigned int m_ebo = 0;
		unsigned int m_vbo = 0;
		unsigned int m_vao = 0;

		glm::vec3 m_color;

	};


}