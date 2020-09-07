#pragma once

#include <vector>
#include <memory>
#include <future>
#include <queue>
#include <set>

#include "Core.h"

#include "Camera.h"
#include "Material.h"

#include "Light/Light.h"

#include "Graphics/Shader.h"
#include "Mesh/Mesh.h"
#include "Mesh/PointMesh.h"

#include "entt.hpp"

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

		void AddLight(std::shared_ptr<Light> light);

		entt::entity CreateEntity();
		void AddTransform(entt::entity entity, const Transform& transform);
		void AddMesh(entt::entity entity, Ref<Mesh> mesh, Ref<Material> material);

		void LoadObject(const std::string& filepath, std::shared_ptr<Material> material, Transform transform);

		void SetCamera(std::shared_ptr<Camera> camera);
		std::shared_ptr<Camera> GetCamera() const;

		void Update();
		void Render();

		size_t GetNumEntities() const { return m_registry.size(); }
		size_t GetNumLights() const { return m_lights.size(); }

		Ref<MeshData> GetCollectiveMeshData();
		std::vector<Ref<Light>> GetLights() const;
		std::vector<Ref<Material>> GetMaterials();

	private:

		void RenderGrid();
		static void StaticLoadObject(std::queue<ObjectUpload>* queue, const std::string filepath, std::shared_ptr<Material> material, Transform transform);

	private:
		entt::registry m_registry{};

		std::shared_ptr<Shader> m_point_shader;

		std::vector<std::shared_ptr<Light>> m_lights;

		std::shared_ptr<Shader> m_shader;

		std::shared_ptr<Camera> m_camera;

		std::vector<std::future<void>> m_Futures{};

		std::mutex m_upload_mutex;
		std::queue<ObjectUpload> m_uploads{};
	};


}