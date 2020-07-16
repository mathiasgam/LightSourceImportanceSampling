#pragma once

#include "glm.hpp"

#include "Transform.h"

namespace LSIS {

	class Camera
	{
	public:
		Camera(glm::vec3 position = { 0,0,0 }, glm::vec3 rotation = { 0,0,0 }, glm::uvec2 resolution = { 720,512 });
		virtual ~Camera();

		void SetPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotation);
		void SetResolution(glm::uvec2 resolution);
		void SetFOV(float fov);

		void LookAt(glm::vec3 position);

		glm::mat4 GetViewProjectionMatrix() const;

	private:
		Transform m_transform;

		glm::uvec2 m_resolution;
		float m_FOV = 90;
	};

}
