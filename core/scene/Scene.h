#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"
#include "Light.h"
#include "Camera.h"

#include "graphics/Shader.h"

namespace LSIS {

	class Scene {
	public:
		Scene();
		virtual ~Scene();

		void AddMesh(std::shared_ptr<Mesh> mesh);
		void AddLight(std::shared_ptr<Light> light);

		void SetCamera(std::shared_ptr<Camera> camera);
		std::shared_ptr<Camera> GetCamera() const;

		void Upload();
		void Render();

		size_t GetNumMeshes() const;
		size_t GetNumLights() const;

	private:
		std::vector<std::shared_ptr<Mesh>> m_meshes;
		std::vector<std::shared_ptr<Light>> m_lights;

		std::shared_ptr<Shader> m_shader;

		std::shared_ptr<Camera> m_camera;

	};

}