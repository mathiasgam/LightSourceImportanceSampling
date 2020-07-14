#include "Scene.h"
#include "mesh/MeshLoader.h"

#include <iostream>

namespace LSIS {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::AddObject(std::shared_ptr<Object> object)
	{
		m_objects.push_back(object);
	}

	void Scene::AddLight(std::shared_ptr<Light> light)
	{
		m_lights.push_back(light);
	}

	void Scene::LoadObject(const std::string& filepath, std::shared_ptr<Material> material, Transform transform)
	{
		m_Futures.push_back(std::async(std::launch::async, StaticLoadObject , &m_uploads, filepath, material, transform));
	}

	void Scene::SetCamera(std::shared_ptr<Camera> camera)
	{
		m_camera = camera;
	}
	std::shared_ptr<Camera> Scene::GetCamera() const
	{
		return m_camera;
	}

	std::mutex queue_mutex;

	void Scene::Update()
	{
		//std::cout << "Scene Update\n";

		queue_mutex.lock();
		while (!m_uploads.empty()) {
			auto& upload = m_uploads.front();

			auto object = std::make_shared<Object>(upload.mesh, upload.material, upload.transform);
			AddObject(object);
			std::cout << "Upload Complete\n";

			m_uploads.pop();
		}
		queue_mutex.unlock();
	}

	void Scene::Render()
	{
		glm::mat4 cam_matrix = m_camera->GetViewProjectionMatrix();
		for (auto& object : m_objects) {
			object->Render(cam_matrix);
		}
	}
	size_t Scene::GetNumObjects() const
	{
		return m_objects.size();
	}
	size_t Scene::GetNumLights() const
	{
		return m_lights.size();
	}

	void Scene::StaticLoadObject(std::queue<ObjectUpload>* queue, const std::string filepath, std::shared_ptr<Material> material, Transform transform)
	{
		auto mesh = MeshLoader::LoadFromOBJ(filepath);
		ObjectUpload upload;
		upload.material = material;
		upload.transform = transform;
		upload.mesh = mesh;
		queue_mutex.lock();
		queue->push(upload);
		queue_mutex.unlock();
		std::cout << "Loaded mesh: " << filepath << std::endl;
	}
}
