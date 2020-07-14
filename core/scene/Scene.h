#pragma once

#include <vector>
#include <memory>
#include <future>
#include <queue>

#include "Object.h"
#include "Light.h"
#include "Camera.h"

#include "Graphics/Shader.h"

namespace LSIS {

	class Scene {

		struct ObjectUpload {
			std::shared_ptr<MeshData> mesh;
			std::shared_ptr<Material> material;
			Transform transform;
		};

	public:
		Scene();
		virtual ~Scene();

		void AddObject(std::shared_ptr<Object> object);
		void AddLight(std::shared_ptr<Light> light);

		void LoadObject(const std::string& filepath, std::shared_ptr<Material> material, Transform transform);

		void SetCamera(std::shared_ptr<Camera> camera);
		std::shared_ptr<Camera> GetCamera() const;

		void Update();
		void Render();

		size_t GetNumObjects() const;
		size_t GetNumLights() const;

	private:

		static void StaticLoadObject(std::queue<ObjectUpload>* queue, const std::string filepath, std::shared_ptr<Material> material, Transform transform);

	private:

		std::vector<std::shared_ptr<Object>> m_objects;
		std::vector<std::shared_ptr<Light>> m_lights;

		std::shared_ptr<Shader> m_shader;

		std::shared_ptr<Camera> m_camera;

		std::vector<std::future<void>> m_Futures{};

		std::mutex m_upload_mutex;
		std::queue<ObjectUpload> m_uploads{};
	};

}