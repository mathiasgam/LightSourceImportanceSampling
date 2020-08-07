#pragma once

#include "Event/Event.h"

#include "glm.hpp"

namespace LSIS {

	namespace Input {
		

		glm::vec4 GetCameraPosition();
		glm::vec4 GetCameraVelocity();
		glm::vec3 GetcameraRotation();
		bool HasCameraMoved();

		void SetCameraPosition(glm::vec4 position);
		void SetCameraRotation(glm::vec3 rotation);

		bool OnEvent(const Event& e);
		void Update(float delta);
	}

}