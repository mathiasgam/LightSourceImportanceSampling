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
#include "AccelerationStructure/SAHBVHStructure.h"

#include "LightStructure/LightStructure.h"
#include "LightStructure/LightTree.h"

#include "IO/Image.h"

namespace LSIS {

	static void accumulate(cl_event e, cl_int event_command_exec_status, void* user_data) {
		cl_ulong* nano_seconds = (cl_ulong*)user_data;
		
		cl_ulong time_start;
		cl_ulong time_end;

		clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
		clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

		*nano_seconds += time_end - time_start;
	}

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

		CompileKernels();
		BuildStructure();

		m_profile_data.device = Compute::GetName(Compute::GetDevice());
		m_profile_data.platform = Compute::GetName(Compute::GetPlatform());
		m_profile_data.width = width;
		m_profile_data.height = height;

		LoadHDRI();
		//CHECK(Compute::GetCommandQueue().finish());
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

	std::vector<float> PathTracer::GetPixelBufferData() const
	{
		std::vector<float> result = std::vector<float>(m_num_pixels * 4);

		auto queue = Compute::GetCommandQueue();
		
		cl_int err = queue.enqueueReadBuffer(m_pixel_buffer.GetBuffer(), CL_TRUE, 0, m_num_pixels * sizeof(cl_float4), result.data());
		if (err != CL_SUCCESS) {
			printf("Failed to read buffer!\n");
		}
		
		return result;
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
		m_program_prepare = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/prepare.cl", { "-I Kernels/" });
		m_kernel_prepare = Compute::CreateKernel(m_program_prepare, "prepare");

		m_program_process = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/process.cl", { "-I Kernels/" });
		m_kernel_process = Compute::CreateKernel(m_program_process, "process_intersections");
		m_kernel_lightsample = Compute::CreateKernel(m_program_process, "process_light_sample");
		m_kernel_process_results = Compute::CreateKernel(m_program_process, "process_results");

		std::vector<std::string> options = { "-I Kernels/" };
		if (use_russian_roulette)
			options.push_back("- D RUSSIAN_ROULETTE");
		
		if (use_naive) {
			options.push_back("-D USE_NAIVE");
		}
		else {
			if (use_solid_angle)
				options.push_back("-D SOLID_ANGLE");
			if (use_lighttree)
				options.push_back("-D USE_LIGHTTREE");
			if (use_min_distance)
				options.push_back("-D MIN_DIST");
			if (use_conditional_attenuation)
				options.push_back("-D AVOID_SINGULARITY");
			if (use_orientation)
				options.push_back("-D USE_ORIENTATION");
			if (use_fast_theta_u)
				options.push_back("-D FAST_THETA_U");

		}
		m_program_shade = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/shade.cl", options);
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
		m_pixel_buffer = TypedBuffer<SHARED::Pixel>(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, num_pixels);

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
		LoadSceneData();

		//BVHBuilder builder = BVHBuilder();
		const auto start = std::chrono::high_resolution_clock::now();

#ifdef USE_LBVH
		LBVHStructure structure = LBVHStructure();
		structure.Build(m_vertex_buffer, m_face_buffer);
#else // Use Binned SAH BVH
		SAHBVHStructure structure = SAHBVHStructure(m_vertex_data, m_face_data, m_num_faces);
#endif // USE_LBVH

		const auto end = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::milli> duration = end - start;
		m_profile_data.time_build_bvh = duration.count();

		m_bvh_buffer = structure.GetNodesBuffer();
		m_bboxes_buffer = structure.GetBoundsBuffer();

		m_bvh.SetBVHBuffer(m_bvh_buffer, m_bboxes_buffer);
		m_bvh.SetGeometryBuffers(m_vertex_buffer, m_face_buffer);

		ResetSamples();

		CHECK(Compute::GetCommandQueue().finish());
	}

	void PathTracer::Prepare()
	{
		uint32_t seed = rand();

		CHECK(m_kernel_prepare.setArg(0, sizeof(cl_uint), &m_image_width));
		CHECK(m_kernel_prepare.setArg(1, sizeof(cl_uint), &m_image_height));
		CHECK(m_kernel_prepare.setArg(2, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_prepare.setArg(3, sizeof(cl_uint), &seed));
		CHECK(m_kernel_prepare.setArg(4, sizeof(cl_float4) * 4, &m_cam_projection));
		CHECK(m_kernel_prepare.setArg(5, m_ray_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(6, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(7, m_throughput_buffer.GetBuffer()));
		CHECK(m_kernel_prepare.setArg(8, m_state_buffer.GetBuffer()));

		cl::Event* e = m_event_queue.GetNextEvent();
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_prepare, 0, cl::NDRange(m_num_concurrent_samples), cl::NullRange, nullptr, e));
		CHECK(e->setCallback(CL_COMPLETE, accumulate, &m_profile_data.time_kernel_prepare));
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
		cl_uint seed = rand();

		
		CHECK(m_kernel_shade.setArg(0, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_shade.setArg(1, sizeof(cl_uint), &num_lights));
		CHECK(m_kernel_shade.setArg(2, sizeof(cl_uint), &m_num_pixels));
		CHECK(m_kernel_shade.setArg(3, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_shade.setArg(4, sizeof(cl_uint), &seed));
		CHECK(m_kernel_shade.setArg(5, m_intersection_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(6, m_geometric_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(7, m_lights.GetBuffer()));
		CHECK(m_kernel_shade.setArg(8, m_material_buffer.GetBuffer()));
		if (use_lighttree) {
			CHECK(m_kernel_shade.setArg(9, m_lighttree_buffer.GetBuffer()));
		}
		else {
			CHECK(m_kernel_shade.setArg(9, m_cdf_power_buffer.GetBuffer()));
		}
		CHECK(m_kernel_shade.setArg(10, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(11, m_throughput_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(12, m_state_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(13, m_light_contribution_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(14, m_ray_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(15, m_occlusion_ray_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(16, m_background_texture));
		//CHECK(m_kernel_shade.setArg(15, m_sampler));

		cl::Event* e = m_event_queue.GetNextEvent();
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade, 0, cl::NDRange(m_num_concurrent_samples), cl::NullRange, nullptr, e));
		CHECK(e->setCallback(CL_COMPLETE, accumulate, &m_profile_data.time_kernel_shade));
	}

	void PathTracer::ProcessOcclusion()
	{
		CHECK(m_kernel_shade_occlusion.setArg(0, m_occlusion_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(1, m_state_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(2, m_light_contribution_buffer.GetBuffer()));
		CHECK(m_kernel_shade_occlusion.setArg(3, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_shade_occlusion.setArg(4, m_result_buffer.GetBuffer()));

		cl::Event* e = m_event_queue.GetNextEvent();
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade_occlusion, 0, cl::NDRange(m_num_concurrent_samples), cl::NullRange, nullptr, e));
		CHECK(e->setCallback(CL_COMPLETE, accumulate, &m_profile_data.time_kernel_process_occlusion));
	}

	void PathTracer::ProcessResults()
	{
		CHECK(m_kernel_process_results.setArg(0, m_result_buffer.GetBuffer()));
		CHECK(m_kernel_process_results.setArg(1, sizeof(cl_uint), &m_num_concurrent_samples));
		CHECK(m_kernel_process_results.setArg(2, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_process_results.setArg(3, m_pixel_buffer.GetBuffer()));

		cl::Event* e = m_event_queue.GetNextEvent();
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process_results, 0, cl::NDRange(m_num_concurrent_samples), cl::NullRange, nullptr, e));
		CHECK(e->setCallback(CL_COMPLETE, accumulate, &m_profile_data.time_kernel_process_results));
	}

	void PathTracer::Reset()
	{
		LoadHDRI();
		CompileKernels();
		m_viewer.CompileKernels();
		m_bvh.Compile();
		BuildStructure();
		ResetSamples();
		m_profile_data.num_primitives = m_num_faces;
		m_profile_data.num_lights = m_num_lights;
		Compute::GetCommandQueue().enqueueWriteBuffer(m_active_count_buffer.GetBuffer(), CL_TRUE, 0, sizeof(cl_uint), &m_num_concurrent_samples);
	}

	void PathTracer::ResetSamples()
	{
		m_num_samples = 0;
	}

	void PathTracer::SetCameraProjection(glm::mat4 projection)
	{
		m_cam_projection = projection;
	}

	void PathTracer::ProcessPass()
	{
		// Don't do anything if not ready
		if (!ready)
			return;

		//PROFILE_SCOPE("PathTracer");
		Prepare();

		//Compute::GetCommandQueue().enqueueWriteBuffer(m_active_count_buffer.GetBuffer(), CL_TRUE, 0, sizeof(cl_uint), &m_num_concurrent_samples);

		for (auto bounce = 0; bounce < 4; bounce++) {
			// Handle bounce
			cl::Event* e = m_event_queue.GetNextEvent();
			m_bvh.Trace(m_ray_buffer, m_intersection_buffer, m_geometric_buffer, m_active_count_buffer, e);
			CHECK(e->setCallback(CL_COMPLETE, accumulate, &m_profile_data.time_kernel_trace));
			//ProcessIntersections();

			// Process bounce and prepare shadow rays
			Shade();

			if (!use_naive) {
				// if the shadow ray is not occluded, the lights contribution is added to the result
				m_bvh.TraceOcclusion(m_occlusion_ray_buffer, m_occlusion_buffer, m_active_count_buffer);
				ProcessOcclusion();
			}
		}

		// copy results into pixelbuffer
		ProcessResults();

		m_num_samples++;
		m_profile_data.samples = m_num_samples;
	}	
	
	void PathTracer::UpdateRenderTexture()
	{
		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		CHECK(Compute::GetCommandQueue().finish());
		m_viewer.Render();
	}

	void PathTracer::SetMethod(Method m)
	{
		if (m == naive) {
			use_naive = true;
			use_lighttree = false;
			use_orientation = false;
			m_profile_data.sampling = "naive";
		}
		else if (m == energy) {
			use_naive = false;
			use_lighttree = false;
			use_orientation = false;
			m_profile_data.sampling = "energy";
		}
		else if (m == lighttree) {
			use_naive = false;
			use_lighttree = true;
			use_orientation = true;
			m_profile_data.sampling = "lighttree";
		}
		else if (m == spatial) {
			use_naive = false;
			use_lighttree = true;
			use_orientation = false;
			m_profile_data.sampling = "spatial";
		}
	}

	void PathTracer::SetClusterAttenuation(ClusterAttenuation atten)
	{
		if (atten == ClusterAttenuation::Center) {
			use_min_distance = false;
			use_conditional_attenuation = false;
			m_profile_data.attenuation = "center";
		}
		else if (atten == ClusterAttenuation::Conditional) {
			use_min_distance = false;
			use_conditional_attenuation = true;
			m_profile_data.attenuation = "conditional";
		}
		else if (atten == ClusterAttenuation::ConditionalMinDist) {
			use_min_distance = true;
			use_conditional_attenuation = true;
			m_profile_data.attenuation = "mindist";
		}
	}

	void PathTracer::UseFastThetaU(bool b)
	{
		use_fast_theta_u = b;
		if (b) {
			m_profile_data.theta_u = "approx";
		}
		else {
			m_profile_data.theta_u = "brute";
		}
	}

	void PathTracer::SetUseHDRI(bool b)
	{
		use_hdri = b;
	}

	inline glm::vec3 convert(cl_float4 in) {
		return glm::vec3(in.x, in.y, in.z);
	}

	void PathTracer::LoadSceneData()
	{
		auto scene = Application::Get()->GetScene();
		Scene* p_scene = scene.get();

		auto entities = scene->GetEntities<MeshComponent, TransformComponent>();

		size_t num_vertices = 0;
		size_t num_indices = 0;
		size_t num_materials = 0;

		if (entities.empty()) {
			m_num_faces = 0;
			m_num_vertices = 0;
			m_num_lights = 0;
			ready = false;
			return;
		}

		for (auto handle : entities) {
			Entity entity = { handle, p_scene };
			auto mesh = entity.GetComponent<MeshComponent>().mesh->GetData();
			auto transform = entity.GetComponent<TransformComponent>().Transform;
			num_vertices += mesh->GetNumVertices();
			num_indices += mesh->GetNumIndices();
			num_materials += mesh->GetNumMaterials();
		}

		size_t num_faces = num_indices;

		std::vector<SHARED::Material> materials_data = std::vector<SHARED::Material>(num_materials);
		std::vector<SHARED::Vertex> vertices_data = std::vector<SHARED::Vertex>(num_vertices);
		std::vector<SHARED::Face> faces_data = std::vector<SHARED::Face>(num_faces);

		size_t num_emissive_faces = 0;
		std::list<int> mesh_light_indices{};

		size_t index_face = 0;
		size_t index_vertex = 0;
		size_t index_material = 0;

		for (auto handle : entities) {
			Entity entity = { handle, p_scene };
			auto mesh = entity.GetComponent<MeshComponent>().mesh->GetData();
			auto transform = entity.GetComponent<TransformComponent>().Transform;
			auto indices = mesh->GetIndices();
			auto vertices = mesh->GetVertices();
			auto materials = mesh->GetMaterials();

			size_t num_faces_object = mesh->GetNumIndices();
			size_t num_vertices_object = mesh->GetNumVertices();
			size_t num_materials_object = mesh->GetNumMaterials();

			for (size_t i = 0; i < num_faces_object; i++) {
				FaceData face = indices[i];
				uint32_t v0 = face.vertex0 + index_vertex;
				uint32_t v1 = face.vertex1 + index_vertex;
				uint32_t v2 = face.vertex2 + index_vertex;
				int idx_mat = face.material + index_material;
				glm::vec3 e = materials[face.material].emissive;
				if (e.x > 0.0f || e.y > 0.0f || e.z > 0.0f) {
					num_emissive_faces++;
					mesh_light_indices.push_back(index_face);
				}
				faces_data[index_face++] = SHARED::make_face(v0, v1, v2, idx_mat);
			}

			for (size_t i = 0; i < num_vertices_object; i++) {
				VertexData v = vertices[i];
				glm::vec4 p = transform * glm::vec4(v.position, 1.0f);
				glm::vec4 n = transform * glm::vec4(v.normal, 0.0f);
				glm::vec3 position = { p.x,p.y,p.z };
				glm::vec3 normal = { n.x,n.y,n.z };
				vertices_data[index_vertex++] = SHARED::make_vertex(position, normal, v.uv);
			}

			for (size_t i = 0; i < num_materials_object; i++) {
				MaterialData data = materials[i];
				materials_data[index_material++] = SHARED::make_material(data.diffuse, data.specular, data.emissive);
			}

		}

		auto context = Compute::GetContext();
		auto queue = Compute::GetCommandQueue();

		m_face_buffer = TypedBuffer<SHARED::Face>(context, CL_MEM_READ_ONLY, num_faces);
		m_vertex_buffer = TypedBuffer<SHARED::Vertex>(context, CL_MEM_READ_ONLY, num_vertices);
		m_material_buffer = TypedBuffer<SHARED::Material>(context, CL_MEM_READ_ONLY, num_materials);

		CHECK(queue.enqueueWriteBuffer(m_face_buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Face) * faces_data.size(), faces_data.data()));
		CHECK(queue.enqueueWriteBuffer(m_vertex_buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Vertex) * vertices_data.size(), vertices_data.data()));
		CHECK(queue.enqueueWriteBuffer(m_material_buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Material) * materials_data.size(), materials_data.data()));

		if (m_vertex_data != nullptr)
			delete[] m_vertex_data;
		if (m_face_data != nullptr)
			delete[] m_face_data;

		m_vertex_data = new SHARED::Vertex[num_vertices];
		m_face_data = new SHARED::Face[num_faces];
		m_num_vertices = num_vertices;
		m_num_faces = num_faces;

		memcpy(m_vertex_data, vertices_data.data(), num_vertices * sizeof(SHARED::Vertex));
		memcpy(m_face_data, faces_data.data(), num_faces * sizeof(SHARED::Face));

		printf("faces: %zd, vertices: %zd, materials: %zd\n", num_faces, num_vertices, num_materials);

		// load scene lights
		//auto scene_lights = scene->GetLights();

		// get the number of lights and allocate the space for the temporary buffer data
		size_t num_lights = 0;
		std::vector<SHARED::Light> lights_data = std::vector<SHARED::Light>(num_lights);

		// format and store scene lights data
		//for (auto i = 0; i < num_lights; i++) {
		//	auto& light = scene_lights[i];
		//	lights_data[i] = SHARED::make_light(light->GetPosition(), { 0,0,0 }, light->GetColor());
		//}

		for (auto i : mesh_light_indices) {
			SHARED::Face face = m_face_data[i];
			glm::vec3 p0 = convert(m_vertex_data[face.index.x].position);
			glm::vec3 p1 = convert(m_vertex_data[face.index.y].position);
			glm::vec3 p2 = convert(m_vertex_data[face.index.z].position);

			const glm::vec3 cross_tan_bitan = glm::cross(p1 - p0, p2 - p0);
			const float area = glm::length(cross_tan_bitan) * 0.5f;
			const glm::vec3 dir = glm::normalize(cross_tan_bitan);


			glm::vec3 n0 = convert(m_vertex_data[face.index.x].normal);
			glm::vec3 n1 = convert(m_vertex_data[face.index.y].normal);
			glm::vec3 n2 = convert(m_vertex_data[face.index.z].normal);

			SHARED::Material mat = materials_data[face.index.w];

			glm::vec3 n = glm::normalize(n0 + n1 + n2);
			glm::vec3 i = convert(mat.emission);
			glm::vec3 p = ((p0 + p1 + p2) / 3.0f);

			//printf("light: [%.2f,%.2f,%.2f]\n", i.x, i.y, i.z);

			//lights_data.push_back(SHARED::make_light(p, n, i));
			lights_data.push_back(SHARED::make_mesh_light(p0, p1, p2, dir, i));
			num_lights++;
		}

		// if no lights are present in the scene, push a empty, light to avoid crashing the kernel
		if (lights_data.empty()) {
			lights_data.push_back(SHARED::make_light({ 0,0,0 }, { 0,1,0 }, { 0,0,0 }));
			num_lights++;
		}

		m_lights = TypedBuffer<SHARED::Light>(context, CL_MEM_READ_ONLY, num_lights);
		if (num_lights > 0)
			CHECK(Compute::GetCommandQueue().enqueueWriteBuffer(m_lights.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Light) * num_lights, lights_data.data()));

		if (!use_naive)
		{
			const auto start = std::chrono::high_resolution_clock::now();

			if (use_lighttree) {
				LightTree light_tree = LightTree(lights_data.data(), num_lights);
				m_lighttree_buffer = light_tree.GetNodeBuffer();
			}
			else {
				m_cdf_power_buffer = build_power_sampling_buffer(lights_data.data(), num_lights);
			}
			const auto end = std::chrono::high_resolution_clock::now();
			const std::chrono::duration<double, std::milli> duration = end - start;
			m_profile_data.time_build_lightstructure = duration.count();
		}

		m_num_lights = num_lights;

		ready = true;
		printf("PointLights: %zd, MeshLights: %zd\n", num_lights, num_emissive_faces);
	}

	void PathTracer::LoadHDRI()
	{
		cl_int err;
		m_sampler = cl::Sampler(Compute::GetContext(), true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, &err);
		if (err) {
			auto err_string = GET_CL_ERROR_CODE(err);
			printf("Error: %s\n", err_string.c_str());
			exit(err);
		}

		float* hdr_data;
		int hdr_width, hdr_height, hdr_channels;
		if (use_hdri) {
			hdr_data = LoadHDRImage("../Assets/Images/HDRIs/kloppenheim_06_2k.hdr", &hdr_width, &hdr_height, &hdr_channels);
		}
		else {
			hdr_width = 1;
			hdr_height = 1;
			hdr_channels = 3;
			hdr_data = new float[3];
			hdr_data[0] = 0.0f;
			hdr_data[1] = 0.0f;
			hdr_data[2] = 0.0f;
		}

		cl::ImageFormat format = {};
		format.image_channel_order = CL_RGBA;
		format.image_channel_data_type = CL_FLOAT;
		m_background_texture = cl::Image2D(Compute::GetContext(), CL_MEM_READ_ONLY, format, hdr_width, hdr_height, 0, nullptr);
		size_t origin[3] = { 0,0,0 };
		size_t region[3] = { hdr_width,hdr_height,1 };

		err = clEnqueueWriteImage(Compute::GetCommandQueue()(), m_background_texture(), CL_TRUE, origin, region, 0, 0, (void*)hdr_data, 0, nullptr, nullptr);
		if (err) {
			auto err_string = GET_CL_ERROR_CODE(err);
			printf("Error: %s\n", err_string.c_str());
			exit(err);
		}


		delete[] hdr_data;
	}

}