#pragma once

#include "glm.hpp"

namespace LSIS {

	class Camera
	{
	public:
		Camera(glm::vec3 position, glm::vec3 direction, glm::uvec2 resolution = { 720,512 });
		virtual ~Camera();

		void SetPosition(glm::vec3 position);
		void SetDirection(glm::vec3 direction);
		void SetResolution(glm::uvec2 resolution);

		void LookAt(glm::vec3 position);

	private:
		glm::vec3 m_position;
		glm::vec3 m_direction;

		glm::uvec2 m_resolution;
	};

}
