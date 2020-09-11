#include "pch.h"
#include "Core/Timer.h"
#include "PathTracer.h"

#include "Core/Application.h"
#include "Event/Event.h"
#include "Input/KeyCodes.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"

#include <list>
#include <tuple>

#include "AccelerationStructure/LBVHStructure.h"
#include "AccelerationStructure/BVHBuilder.h"

#include "IO/Image.h"

namespace LSIS {


	PathTracer::PathTracer(uint32_t width, uint32_t height) :
		m_image_width(width),
		m_image_height(height),
		m_num_pixels(width* height),
		m_num_rays(width* height),
		m_viewer(width, height),
		m_bvh()
	{
		printf("resolution: [%d,%d]", width, height);

		PrepareCameraRays(Compute::GetContext());
		SetEventCategoryFlags(EventCategory::EventCategoryApplication | EventCategory::EventCategoryKeyboard);

		CompileKernels();
		BuildStructure();

		cl_int err;
		m_sampler = cl::Sampler(Compute::GetContext(), true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, &err);
		if (err) {
			auto err_string = GET_CL_ERROR_CODE(err);
			printf("Error: %s\n", err_string.c_str());
			exit(err);
		}

		int hdr_width, hdr_height, hdr_channels;
		float* hdr_data = LoadHDRImage("../Assets/Images/HDRIs/kloppenheim_06_2k.hdr", &hdr_width, &hdr_height, &hdr_channels);
		/*
		const float pixels[] = {
			1.0f,1.0f,1.0f,1.0f,
			1.0f,0.0f,0.0f,1.0f,
			0.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,1.0f
		};
		*/

		//m_background_texture = cl::Image2D(image);
		cl::ImageFormat format = {};
		format.image_channel_order = CL_RGBA;
		format.image_channel_data_type = CL_FLOAT;
		m_background_texture = cl::Image2D(Compute::GetContext(), CL_MEM_READ_ONLY, format, hdr_width, hdr_height, 0, nullptr);
		size_t origin[3] = { 0,0,0 };
		size_t region[3] = { hdr_width,hdr_height,1 };
		/*
		err = clEnqueueWriteImage(Compute::GetCommandQueue()(), image, CL_TRUE, origin, region, 0, 0, (void*)pixels, 0, nullptr, nullptr);
		if (err) {
			auto err_string = GET_CL_ERROR_CODE(err);
			printf("Error: %s\n", err_string.c_str());
			exit(err);
		}
		*/

		err = clEnqueueWriteImage(Compute::GetCommandQueue()(), m_background_texture(), CL_TRUE, origin, region, 0, 0, (void*)hdr_data, 0, nullptr, nullptr);
		if (err) {
			auto err_string = GET_CL_ERROR_CODE(err);
			printf("Error: %s\n", err_string.c_str());
			exit(err);
		}
	}

	PathTracer::~PathTracer()
	{
	}

	void PathTracer::SetImageSize(const uint32_t width, const uint32_t height)
	{
		m_image_width = width;
		m_image_height = height;

		PrepareCameraRays(Compute::GetContext());
	}

	void PathTracer::OnUpdate(float delta)
	{
		PROFILE_SCOPE("PathTracer");
		Prepare();

		Compute::GetCommandQueue().enqueueWriteBuffer(m_active_count_buffer.GetBuffer(), CL_TRUE, 0, sizeof(cl_uint), &m_num_concurrent_samples);

		for (auto bounce = 0; bounce < 4; bounce++) {
			// Handle bounce
			m_bvh.Trace(m_ray_buffer, m_intersection_buffer, m_geometric_buffer, m_active_count_buffer);
			//ProcessIntersections();

			// Process bounce and prepare shadow rays
			Shade();

			// if the shadow ray is not occluded, the lights contribution is added to the result
			m_bvh.TraceOcclusion(m_occlusion_ray_buffer, m_occlusion_buffer, m_active_count_buffer);
			ProcessOcclusion();
		}

		// copy results into pixelbuffer
		ProcessResults();

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		m_viewer.Render();

		Compute::GetCommandQueue().finish();

		m_num_samples++;
		printf("Num Samples %d, ", m_num_samples);
	}

	bool PathTracer::OnEvent(const Event& e)
	{
		if (e.GetEventType() == EventType::KeyPressed) {
			auto key_event = (const KeyPressedEvent&)e;
			auto key = key_event.GetKey();
			if (key == KEY_B) {
				std::cout << "PT Event: " << e << std::endl;
				BuildStructure();
				return true;
			}
			else if (key == KEY_R) {
				CompileKernels();
				m_viewer.CompileKernels();
				m_bvh.Compile();
				std::cout << "PT: Kernels Recompiled!\n";
				return true;
			}
		}
		if (e.GetEventType() == EventType::CameraUpdated) {
			ResetSamples();
			std::cout << "Cam update\n";
			return true;
		}
		return false;
	}

	void PathTracer::OnAttach()
	{

	}

	void PathTracer::OnDetach()
	{
	}

	size_t PathTracer::CalculateMemory() const
	{
		size_t mem_size = 0;

		mem_size += sizeof(SHARED::Pixel) * m_num_pixels;
		mem_size += sizeof(SHARED::Ray) * m_num_rays;
		mem_size += sizeof(SHARED::Intersection) * m_num_rays;
		//mem_size += m_ray_buffer.Size();
		//mem_size += m_intersection_buffer.Size();
		//mem_size += m_pixel_buffer.Size();

		mem_size += m_viewer.CalculateMemory();


		return mem_size;
	}

	void PathTracer::CompileKernels()
	{
		m_program_prepare = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/prepare.cl", { "Kernels/" });
		m_kernel_prepare = Compute::CreateKernel(m_program_prepare, "prepare");

		m_program_process = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/process.cl", { "Kernels/" });
		m_kernel_process = Compute::CreateKernel(m_program_process, "process_intersections");
		m_kernel_lightsample = Compute::CreateKernel(m_program_process, "process_light_sample");
		m_kernel_process_results = Compute::CreateKernel(m_program_process, "process_results");

		m_program_shade = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/shade.cl", { "Kernels/" });
		m_kernel_shade = Compute::CreateKernel(m_program_shade, "ProcessBounce");
		m_kernel_shade_occlusion = Compute::CreateKernel(m_program_shade, "shade_occlusion");
	}

	void PathTracer::PrepareCameraRays(const cl::Context& context)
	{
		size_t num_pixels = static_cast<size_t>(m_image_width)* static_cast<size_t>(m_image_height);
		size_t num_concurrent_samples = num_pixels * m_num_samples_per_pixel;

		m_state_buffer = TypedBuffer<cl_int>(context, CL_READ_WRITE_CACHE, num_concurrent_samples);
		m_result_buffer = TypedBuffer<cl_float3>(context, CL_READ_WRITE_CACHE, num_concurrent_samples);
		m_throughput_buffer = TypedBuffer<cl_float3>(context, CL_READ_WRITE_CACHE, num_concurrent_samples);
		m_depth_buffer = TypedBuffer<cl_float>(context, CL_READ_WRITE_CACHE, num_concurrent_samples);
		//m_sample_buffer = TypedBuffer<SHARED::Sample>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_geometric_buffer = TypedBuffer<SHARED::GeometricInfo>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_pixel_buffer = TypedBuffer<SHARED::Pixel>(context, CL_MEM_READ_WRITE, num_pixels);

		m_source_buffer = TypedBuffer<cl_uint>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_active_count_buffer = TypedBuffer<cl_uint>(context, CL_MEM_READ_WRITE, 1);

		m_light_contribution_buffer = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_concurrent_samples);

		m_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_intersection_buffer = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_occlusion_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_occlusion_buffer = TypedBuffer<cl_int>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
	}

	void PathTracer::BuildStructure()
	{
		//LoadMaterials();
		LoadGeometry();
		LoadLights();


		//BVHBuilder builder = BVHBuilder();

		LBVHStructure structure = LBVHStructure();
		structure.Build(m_vertex_buffer, m_face_buffer);

		//auto context = Compute::GetContext();

		//m_vertex_buffer = structure.GetVertices();
		//m_face_buffer = structure.GetFaces();
		m_bvh_buffer = structure.GetNodes();
		m_bboxes_buffer = structure.GetBBoxes();

		//auto pair = builder.Build(m_vertex_buffer, m_face_buffer);
		//m_bvh_buffer = pair.first;
		//m_bboxes_buffer = pair.second;

		m_bvh.SetBVHBuffer(m_bvh_buffer, m_bboxes_buffer);
		m_bvh.SetGeometryBuffers(m_vertex_buffer, m_face_buffer);

		ResetSamples();
	}

	void PathTracer::Prepare()
	{
		auto cam = Application::Get()->GetScene()->GetCamera();
		glm::mat4 cam_matrix = glm::transpose(glm::inverse(cam->GetViewProjectionMatrix()));

		CHECK(m_kernel_prepare.setArg(0, sizeof(cl_uint), &m_image_width));
		CHECK(m_kernel_prepare.setArg(1, sizeof(cl_uint), &m_image_height));
		CHECK(m_kernel_prepare.setArg(2, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_prepare.setArg(3, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_prepare.setArg(4, sizeof(cl_float4) * 4, &cam_matrix));
		CHECK(m_kernel_prepare.setArg(5, m_ray_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(6, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(7, m_throughput_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(8, m_state_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_prepare, 0, cl::NDRange(m_num_concurrent_samples)));
	}

	void PathTracer::ProcessIntersections()
	{
		cl_uint num_rays = m_image_width * m_image_height;
		cl_uint num_vertices = static_cast<cl_uint>(m_vertex_buffer.Count());
		cl_uint num_faces = static_cast<cl_uint>(m_face_buffer.Count());

		CHECK(m_kernel_process.setArg(0, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_process.setArg(1, sizeof(cl_uint), &num_rays));
		CHECK(m_kernel_process.setArg(2, sizeof(cl_uint), &num_vertices));
		CHECK(m_kernel_process.setArg(3, sizeof(cl_uint), &num_faces));
		CHECK(m_kernel_process.setArg(4, m_vertex_buffer.GetBuffer()));
		CHECK(m_kernel_process.setArg(5, m_face_buffer.GetBuffer()));
		CHECK(m_kernel_process.setArg(6, m_ray_buffer.GetBuffer()));
		CHECK(m_kernel_process.setArg(7, m_intersection_buffer.GetBuffer()));
		CHECK(m_kernel_process.setArg(8, m_state_buffer.GetBuffer()));
		//CHECK(m_kernel_process.setArg(9, m_sample_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process, 0, cl::NDRange(num_rays)));
	}

	void PathTracer::Shade()
	{
		cl_uint num_lights = static_cast<cl_uint>(m_lights.Count());

		CHECK(m_kernel_shade.setArg(0, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_shade.setArg(1, sizeof(cl_uint), &num_lights));
		CHECK(m_kernel_shade.setArg(2, sizeof(cl_uint), &m_num_pixels));
		CHECK(m_kernel_shade.setArg(3, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_shade.setArg(4, m_intersection_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(5, m_geometric_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(6, m_lights.GetBuffer()));
		CHECK(m_kernel_shade.setArg(7, m_material_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(8, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(9, m_throughput_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(10, m_state_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(11, m_light_contribution_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(12, m_ray_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(13, m_occlusion_ray_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(14, m_background_texture));
		//CHECK(m_kernel_shade.setArg(15, m_sampler));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade, 0, cl::NDRange(m_num_concurrent_samples)));
	}

	void PathTracer::ProcessOcclusion()
	{
		CHECK(m_kernel_shade_occlusion.setArg(0, m_occlusion_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(1, m_state_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(2, m_light_contribution_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(3, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_shade_occlusion.setArg(4, m_result_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade_occlusion, 0, cl::NDRange(m_num_concurrent_samples)));
	}

	void PathTracer::ProcessResults()
	{
		CHECK(m_kernel_process_results.setArg(0, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_process_results.setArg(1, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_process_results.setArg(2, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_process_results.setArg(3, m_pixel_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process_results, 0, cl::NDRange(m_num_concurrent_samples)));
	}

	void PathTracer::ResetSamples()
	{
		m_num_samples = 0;
	}

	void PathTracer::LoadGeometry()
	{
		auto scene = Application::Get()->GetScene();
		Scene* p_scene = scene.get();
		
		auto entities = scene->GetEntities<MeshComponent, TransformComponent>();

		size_t num_vertices = 0;
		size_t num_indices = 0;
		size_t num_materials = 0;
		std::unordered_map<Ref<Material>, uint32_t> material_index_map{};
		std::list<std::tuple<Ref<MeshData>, glm::mat4, Ref<Material>>> objects{};

		for (auto handle : entities) {
			Entity entity = { handle, p_scene };
			auto mesh = entity.GetComponent<MeshComponent>().mesh->GetData();
			auto transform = entity.GetComponent<TransformComponent>().Transform;
			num_vertices += mesh->GetNumVertices();
			num_indices += mesh->GetNumIndices();

			auto material = entity.GetComponent<MeshComponent>().material;
			if (material_index_map.find(material) == material_index_map.end()) {
				material_index_map.insert({ material, num_materials++ });
			}

			objects.push_back(std::make_tuple(mesh, transform, material));
		}

		size_t num_faces = num_indices / 3;

		std::vector<SHARED::Material> materials_data = std::vector<SHARED::Material>(num_materials);
		std::vector<SHARED::Vertex> vertices_data = std::vector<SHARED::Vertex>(num_vertices);
		std::vector<SHARED::Face> faces_data = std::vector<SHARED::Face>(num_faces);

		for (auto index : material_index_map) {
			auto mat = index.first;
			auto id = index.second;
			materials_data[id] = SHARED::make_material(mat->GetDiffuse(), mat->GetSpecular());
		}

		size_t index_face = 0;
		size_t index_vertex = 0;
		for (auto& [mesh, transform, material] : objects) {
			auto indices = mesh->GetIndices();
			auto vertices = mesh->GetVertices();

			cl_uint material_index = material_index_map[material];

			size_t num_faces_object = mesh->GetNumIndices() / 3;
			size_t num_vertices_object = mesh->GetNumVertices();

			for (size_t i = 0; i < num_faces_object; i++) {
				uint32_t v0 = indices[i * 3 + 0] + index_vertex;
				uint32_t v1 = indices[i * 3 + 1] + index_vertex;
				uint32_t v2 = indices[i * 3 + 2] + index_vertex;
				faces_data[index_face++] = SHARED::make_face(v0, v1, v2, material_index);
			}

			for (size_t i = 0; i < num_vertices_object; i++) {
				VertexData v = vertices[i];
				glm::vec4 p = transform * glm::vec4(v.position, 1.0f);
				glm::vec4 n = transform * glm::vec4(v.normal, 0.0f);
				glm::vec3 position = { p.x,p.y,p.z };
				glm::vec3 normal = { n.x,n.y,n.z };
				vertices_data[index_vertex++] = SHARED::make_vertex(position, normal, v.uv);
			}
		}

		auto queue = Compute::GetCommandQueue();

		std::vector<cl::Event> write_events = std::vector<cl::Event>(3);
		write_events[0] = cl::Event();
		write_events[1] = cl::Event();
		write_events[2] = cl::Event();

		m_face_buffer = TypedBuffer<SHARED::Face>(Compute::GetContext(), CL_MEM_READ_ONLY, num_faces);
		m_vertex_buffer = TypedBuffer<SHARED::Vertex>(Compute::GetContext(), CL_MEM_READ_ONLY, num_vertices);
		m_material_buffer = TypedBuffer<SHARED::Material>(Compute::GetContext(), CL_MEM_READ_ONLY, num_materials);

		queue.enqueueWriteBuffer(m_face_buffer.GetBuffer(), CL_FALSE, 0, sizeof(SHARED::Face) * faces_data.size(), faces_data.data(), nullptr, &write_events[0]);
		queue.enqueueWriteBuffer(m_vertex_buffer.GetBuffer(), CL_FALSE, 0, sizeof(SHARED::Vertex) * vertices_data.size(), vertices_data.data(), nullptr, &write_events[1]);
		queue.enqueueWriteBuffer(m_material_buffer.GetBuffer(), CL_FALSE, 0, sizeof(SHARED::Material) * materials_data.size(), materials_data.data(), nullptr, &write_events[2]);

		// Wait for data to be uploaded
		cl::WaitForEvents(write_events);

		printf("indices: %zd, vertices: %zd, materials: %zd\n", num_indices, num_vertices, num_materials);
	}

	void PathTracer::LoadMaterials()
	{
		auto scene_materials = Application::Get()->GetScene()->GetMaterials();

		const size_t num_materials = scene_materials.size();
		std::vector<SHARED::Material> material_data = std::vector<SHARED::Material>(num_materials);

		for (auto i = 0; i < num_materials; i++) {
			auto& material = scene_materials[i];
			material_data[i] = SHARED::make_material(material->GetDiffuse(), material->GetSpecular());
		}

		m_material_buffer = TypedBuffer<SHARED::Material>(Compute::GetContext(), CL_MEM_READ_ONLY, num_materials);
		Compute::GetCommandQueue().enqueueWriteBuffer(m_material_buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Material) * num_materials, material_data.data());

		printf("Num Materials: %zd\n", num_materials);
	}

	void PathTracer::LoadLights()
	{
		// load scene lights
		auto app = Application::Get();
		auto scene_lights = app->GetScene()->GetLights();

		// get the number of lights and allocate the space for the temporary buffer data
		const size_t num_lights = scene_lights.size();
		std::vector<SHARED::Light> lights_data = std::vector<SHARED::Light>(num_lights);

		// format and store scene lights data
		for (auto i = 0; i < num_lights; i++) {
			auto& light = scene_lights[i];
			lights_data[i] = SHARED::make_light(light->GetPosition(), { 0,0,0 }, light->GetColor());
		}

		m_lights = TypedBuffer<SHARED::Light>(Compute::GetContext(), CL_MEM_READ_ONLY, num_lights);
		Compute::GetCommandQueue().enqueueWriteBuffer(m_lights.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Light) * num_lights, lights_data.data());

		printf("Num lights: %zd\n", num_lights);
	}

}