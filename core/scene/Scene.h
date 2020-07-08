#pragma once

#include <vector>
#include <memory>

#include "Object.h"
#include "Light.h"
#include "Camera.h"

#include "graphics/Shader.h"

namespace LSIS {

	class Scene {
	public:
		Scene();
		virtual ~Scene();

		void AddObject(std::shared_ptr<Object> object);
		void AddLight(std::shared_ptr<Light> light);

		void SetCamera(std::shared_ptr<Camera> camera);
		std::shared_ptr<Camera> GetCamera() const;

		void Render();

		size_t GetNumObjects() const;
		size_t GetNumLights() const;

	private:
		std::vector<std::shared_ptr<Object>> m_objects;
		std::vector<std::shared_ptr<Light>> m_lights;

		std::shared_ptr<Shader> m_shader;

		std::shared_ptr<Camera> m_camera;

	};

}