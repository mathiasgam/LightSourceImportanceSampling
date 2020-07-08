#include "Scene.h"

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
	void Scene::SetCamera(std::shared_ptr<Camera> camera)
	{
		m_camera = camera;
	}
	std::shared_ptr<Camera> Scene::GetCamera() const
	{
		return m_camera;
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
}
