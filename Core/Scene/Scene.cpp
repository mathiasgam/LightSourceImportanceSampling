#include "pch.h"
#include "Scene.h"

#include "Mesh/MeshLoader.h"

#include "Graphics/RenderCommand.h"
#include "Graphics/PointRenderer.h"
#include "Graphics/Renderer2D.h"

#include <iostream>
#include <set>

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
		m_Futures.push_back(std::async(std::launch::async, StaticLoadObject, &m_uploads, filepath, material, transform));
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
		LineRenderer::ResetStats();
		LineRenderer::BeginBatch();

		std::vector<glm::vec3> path{};
		path.push_back({ 0,0,0 });
		for (auto& light : m_lights) {
			PointRenderer::DrawPoint(light->GetPosition(), light->GetColor());
			path.push_back(light->GetPosition());
		}

		LineRenderer::DrawLines(path.data(), path.size(), { 1,0,0 });

		RenderGrid();


		PointRenderer::EndBatch();
		PointRenderer::Flush();

		LineRenderer::EndBatch();
		LineRenderer::Flush();

		//m_point_shader->UploadUniformMat4("cam_matrix", cam_matrix);
		//m_points->Bind();
		//RenderCommand::RenderPointMesh(m_points);
	}



	Ref<MeshData> Scene::GetCollectiveMeshData()
	{
		size_t num_vertices = 0;
		size_t num_indices = 0;
		std::vector<VertexData> vertices{};
		std::vector<uint32_t> indices{};
		
		// Find size of all mesh data
		auto view = m_registry.view<MeshComponent, TransformComponent>();
		for (auto entity : view) {
			auto& [mesh,Transform] = view.get<MeshComponent, TransformComponent>(entity);
			num_vertices += mesh.mesh->GetNumVertices();
			num_indices += mesh.mesh->GetNumIndices();
		}
		
		// resize vectors
		vertices.resize(num_vertices);
		indices.resize(num_indices);

		// Read data
		uint32_t index_idx = 0;
		uint32_t index_vtx = 0;
		for (auto entity : view) {
			auto& [mesh, transform] = view.get<MeshComponent, TransformComponent>(entity);
			auto data = mesh.mesh->GetData();

			auto vertex_data = data->GetVertices();
			auto index_data = data->GetIndices();

			for (size_t i = 0; i < data->GetNumIndices(); i++) {
				indices[index_idx++] = index_data[i] + index_vtx;
			}

			for (size_t i = 0; i < data->GetNumVertices(); i++) {
				glm::vec4 p = transform.Transform * glm::vec4(vertex_data[i].position, 1.0f);
				glm::vec4 n = transform.Transform * glm::vec4(vertex_data[i].normal, 0.0f);
				VertexData d{};
				d.position[0] = p.x;
				d.position[1] = p.y;
				d.position[2] = p.z;
				d.normal[0] = n.x;
				d.normal[1] = n.y;
				d.normal[2] = n.z;
				d.uv[0] = vertex_data[i].uv[0];
				d.uv[1] = vertex_data[i].uv[1];
				vertices[index_vtx++] = d;
			}
		}
		

		return std::make_shared<MeshData>(vertices, indices);
	}

	std::vector<Ref<Light>> Scene::GetLights() const
	{
		// return copy of the lights array
		return std::vector<Ref<Light>>(m_lights);
	}

	std::vector<Ref<Material>> Scene::GetMaterials()
	{
		// set of unique materials
		std::set<Ref<Material>> unique_materials = std::set<Ref<Material>>();

		// iterate over all entities with mesh components and save the materials in the set
		auto view = m_registry.view<MeshComponent>();
		for (auto entity : view) {
			auto& mesh = view.get<MeshComponent>(entity);
			unique_materials.insert(mesh.material);
		}


		// allocate return vector
		std::vector<Ref<Material>> materials = std::vector<Ref<Material>>(unique_materials.size());
		
		// insert unique materials into vector
		size_t count = 0;
		for (auto& mat : unique_materials) {
			materials[count++] = mat;
		}

		return materials;
	}

	void Scene::RenderGrid()
	{
		glm::vec3 center = { 0,0,0 };
		glm::vec3 color = { 0.2,0.2,0.2 };
		float d = 1.0f;
		int n = 6;

		const float b = d * n;
		LineRenderer::DrawLine({ -b,0,0 }, { b,0,0 }, color);
		LineRenderer::DrawLine({ 0,0,-b }, { 0,0,b }, color);

		for (int i = 0; i < n; i++) {
			const float a = d * (i + 1);
			LineRenderer::DrawLine({ a, 0,-b }, { a, 0,b }, color);
			LineRenderer::DrawLine({ -a, 0,-b }, { -a, 0,b }, color);
			LineRenderer::DrawLine({ -b, 0,a }, { b, 0,a }, color);
			LineRenderer::DrawLine({ -b, 0,-a }, { b, 0,-a }, color);
		}
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
