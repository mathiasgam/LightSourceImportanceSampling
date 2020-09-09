#include "pch.h"
#include "Core/Timer.h"
#include "PathTracer.h"

#include "Core/Application.h"
#include "Event/Event.h"
#include "Input/KeyCodes.h"

#include "AccelerationStructure/LBVHStructure.h"
#include "AccelerationStructure/BVHBuilder.h"

namespace LSIS {


	PathTracer::PathTracer(uint32_t width, uint32_t height) :
		m_image_width(width),
		m_image_height(height),
		m_num_pixels(width* height),
		m_num_rays(width* height),
		m_viewer(width, height),
		m_bvh()
	{
		PrepareCameraRays(Compute::GetContext());
		SetEventCategoryFlags(EventCategory::EventCategoryApplication | EventCategory::EventCategoryKeyboard);

		CompileKernels();
		BuildStructure();
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

		for (auto bounce = 0; bounce < 4; bounce++) {
			// Handle bounce
			m_bvh.Trace(m_ray_buffer, m_intersection_buffer, m_geometric_buffer);
			//ProcessIntersections();

			// Process bounce and prepare shadow rays
			Shade();

			// if the shadow ray is not occluded, the lights contribution is added to the result
			m_bvh.TraceOcclusion(m_occlusion_ray_buffer, m_occlusion_buffer);
			ProcessOcclusion();
		}

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
				m_num_samples = 0;
				return true;
			}
			else if (key == KEY_R) {
				CompileKernels();
				m_viewer.CompileKernels();
				m_bvh.Compile();
				BuildStructure();
				std::cout << "PT: Kernels Recompiled!\n";
				return true;
			}
		}
		if (e.GetEventType() == EventType::CameraUpdated) {
			ResetSamples();
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

		m_program_shade = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/shade.cl", { "Kernels/" });
		m_kernel_shade = Compute::CreateKernel(m_program_shade, "ProcessBounce");
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

		m_light_contribution_buffer = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_concurrent_samples);

		m_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_intersection_buffer = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_occlusion_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
		m_occlusion_buffer = TypedBuffer<cl_int>(context, CL_MEM_READ_WRITE, num_concurrent_samples);
	}

	void PathTracer::BuildStructure()
	{
		LoadMaterials();
		LoadLights();

		auto app = Application::Get();
		auto geometry = app->GetScene()->GetCollectiveMeshData();

		auto num_vertices = geometry->GetNumVertices();
		auto num_indices = geometry->GetNumIndices();

		//BVHBuilder builder = BVHBuilder();

		LBVHStructure structure = LBVHStructure();
		structure.Build(geometry->GetVertices(), num_vertices, geometry->GetIndices(), num_indices);

		auto context = Compute::GetContext();

		m_vertex_buffer = structure.GetVertices();
		m_face_buffer = structure.GetFaces();
		m_bvh_buffer = structure.GetNodes();
		m_bboxes_buffer = structure.GetBBoxes();

		//auto pair = builder.Build(m_vertex_buffer, m_face_buffer);
		//m_bvh_buffer = pair.first;
		//m_bboxes_buffer = pair.second;

		m_bvh.SetBVHBuffer(m_bvh_buffer, m_bboxes_buffer);
		m_bvh.SetGeometryBuffers(m_vertex_buffer, m_face_buffer);


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
		CHECK(m_kernel_shade.setArg(14, m_pixel_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade, 0, cl::NDRange(m_num_concurrent_samples)));
	}

	void PathTracer::ProcessOcclusion()
	{
	}

	void PathTracer::ResetSamples()
	{
		m_num_samples = 0;
	}

	void PathTracer::LoadGeometry()
	{
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