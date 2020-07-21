#include "pch.h"
#include "Scene.h"

#include "Mesh/MeshLoader.h"

#include "Graphics/RenderCommand.h"
#include "Graphics/PointRenderer.h"

#include <iostream>

#include "Components.h"

namespace LSIS {

	Scene::Scene()
	{
		m_point_shader = Shader::Create("../Assets/Shaders/point.vert", "../Assets/Shaders/point.frag");
	}

	Scene::~Scene()
	{
	}

	void Scene::AddLight(std::shared_ptr<Light> light)
	{
		m_lights.push_back(light);
	}

	entt::entity Scene::CreateEntity()
	{
		return m_registry.create();
	}

	void Scene::AddTransform(entt::entity entity, const Transform& transform)
	{
		const glm::mat4 mat = transform.GetModelMatrix();
		m_registry.emplace<TransformComponent>(entity, mat);
	}

	void Scene::AddMesh(entt::entity entity, Ref<Mesh> mesh, Ref<Material> material)
	{
		m_registry.emplace<MeshComponent>(entity, mesh, material);
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
		queue_mutex.lock();
		while (!m_uploads.empty()) {
			auto& upload = m_uploads.front();

			auto entity = m_registry.create();
			auto mesh = std::make_shared<Mesh>(upload.mesh);

			m_registry.emplace<TransformComponent>(entity, upload.transform.GetModelMatrix());
			m_registry.emplace<MeshComponent>(entity, mesh, upload.material);

			std::cout << "Upload Complete\n";

			m_uploads.pop();
		}
		queue_mutex.unlock();
	}

	void Scene::Render()
	{
		glm::mat4 cam_matrix = m_camera->GetViewProjectionMatrix();
		/*for (auto& object : m_objects) {
			object->Render(cam_matrix);
		}*/

		auto view = m_registry.view<MeshComponent, TransformComponent>();
		for (auto entity : view) {
			auto& [mesh, transform] = view.get<MeshComponent, TransformComponent>(entity);

			mesh.material->Bind(transform.Transform, cam_matrix);
			mesh.mesh->Bind();
			RenderCommand::RenderGeometryBuffer(mesh.mesh);
		}
		

		m_point_shader->Bind();
		m_point_shader->UploadUniformMat4("cam_matrix", cam_matrix);
		
		PointRenderer::ResetStats();
		PointRenderer::BeginBatch();

		for (auto& light : m_lights) {
			PointRenderer::DrawPoint(light->GetPosition(), light->GetColor());
		}
		PointRenderer::EndBatch();
		PointRenderer::Flush();

		//m_point_shader->UploadUniformMat4("cam_matrix", cam_matrix);
		//m_points->Bind();
		//RenderCommand::RenderPointMesh(m_points);
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
