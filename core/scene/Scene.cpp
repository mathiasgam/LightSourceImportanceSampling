#include "Scene.h"

namespace LSIS {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::AddMesh(std::shared_ptr<Mesh> mesh)
	{
		m_meshes.push_back(mesh);
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
	void Scene::Upload()
	{
		m_shader = Shader::Create("kernels/flat.vert", "kernels/flat.frag");
		for (auto& mesh : m_meshes) {
			mesh->Upload();
		}
	}
	void Scene::Render()
	{
		m_shader->Bind();
		for (auto& mesh : m_meshes) {
			m_shader->UploadUniformfloat3("color", mesh->GetColor());
			mesh->Render();
		}
		m_shader->Unbind();
	}
	size_t Scene::GetNumMeshes() const
	{
		return m_meshes.size();
	}
	size_t Scene::GetNumLights() const
	{
		return m_lights.size();
	}
}
