#include "pch.h"
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
		m_camera(width, height),
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
		m_camera.GenerateRays(m_ray_buffer, m_sample_buffer);

		buffer_switch = true;
		m_bvh.Trace(m_ray_buffer, m_intersection_buffer);
		ProcessIntersections();

		Shade();

		/*
		buffer_switch = false;
		m_bvh.Trace(m_ray_bufferB, m_intersection_bufferB);
		ProcessIntersections();
		*/

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		m_viewer.Render();

		m_num_samples++;
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
				m_camera.CompileKernels();
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

		mem_size += m_camera.CalculateMemory();
		mem_size += m_viewer.CalculateMemory();


		return mem_size;
	}

	void PathTracer::CompileKernels()
	{
		m_program_process = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/process.cl", { "Kernels/" });
		m_kernel_process = Compute::CreateKernel(m_program_process, "process_intersections");
		m_kernel_lightsample = Compute::CreateKernel(m_program_process, "process_light_sample");

		m_program_shade = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/shade.cl", { "Kernels/" });
		m_kernel_shade = Compute::CreateKernel(m_program_shade, "shade");
	}

	void PathTracer::TraceRays()
	{
		m_camera.GenerateRays(m_ray_buffer, m_sample_buffer);

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		//m_tracing_structure->TraceRays(m_ray_buffer, m_intersection_buffer);
	}

	void PathTracer::PrepareCameraRays(const cl::Context& context)
	{
		size_t num_pixels = m_image_width * m_image_height;

		m_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_pixels);
		m_intersection_buffer = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_pixels);
		m_pixel_buffer = TypedBuffer<SHARED::Pixel>(context, CL_MEM_READ_WRITE, num_pixels);
		m_sample_buffer = TypedBuffer<SHARED::Sample>(context, CL_MEM_READ_WRITE, num_pixels);
	}

	void PathTracer::BuildStructure()
	{
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

		LoadLights();
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
		CHECK(m_kernel_process.setArg(8, m_sample_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process, 0, cl::NDRange(num_rays)));
	}

	void PathTracer::Shade()
	{
		cl_uint num_samples = static_cast<cl_uint>(m_sample_buffer.Count());
		cl_uint num_lights = static_cast<cl_uint>(m_lights.Count());

		CHECK(m_kernel_shade.setArg(0, sizeof(cl_uint), &num_samples));
		CHECK(m_kernel_shade.setArg(1, sizeof(cl_uint), &num_lights));
		CHECK(m_kernel_shade.setArg(2, sizeof(cl_uint), &m_num_pixels));
		CHECK(m_kernel_shade.setArg(3, sizeof(cl_uint), &m_num_samples));
		CHECK(m_kernel_shade.setArg(4, m_sample_buffer.GetBuffer()));
		CHECK(m_kernel_shade.setArg(5, m_lights.GetBuffer()));
		CHECK(m_kernel_shade.setArg(6, m_pixel_buffer.GetBuffer()));

		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_shade, 0, cl::NDRange(num_samples)));
	}

	void PathTracer::ResetSamples()
	{
		m_num_samples = 0;
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

		printf("Num lights: %d\n", num_lights);
	}

}