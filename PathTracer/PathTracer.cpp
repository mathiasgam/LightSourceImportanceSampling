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
		m_camera.GenerateRays(m_ray_bufferA, m_intersection_bufferA);

		m_bvh.Trace(m_ray_bufferA, m_intersection_bufferA);

		ProcessIntersections();

		//m_bvh.Trace(m_ray_bufferB, m_intersection_bufferB);
		//ProcessIntersections();

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
			std::cout << "Cam Update\n";
			m_num_samples = 0;
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
	}

	void PathTracer::TraceRays()
	{
		m_camera.GenerateRays(m_ray_bufferA, m_intersection_bufferA);

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		//m_tracing_structure->TraceRays(m_ray_buffer, m_intersection_buffer);
	}

	void PathTracer::PrepareCameraRays(const cl::Context& context)
	{
		size_t num_pixels = m_image_width * m_image_height;

		m_ray_bufferA = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_pixels);
		m_ray_bufferB = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_pixels);
		m_intersection_bufferA = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_pixels);
		m_intersection_bufferB = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_pixels);
		m_pixel_buffer = TypedBuffer<SHARED::Pixel>(context, CL_MEM_READ_WRITE, num_pixels);
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

	}

	void PathTracer::ProcessIntersections()
	{
		cl_uint num_rays = m_image_width * m_image_height;
		cl_uint num_vertices = static_cast<cl_uint>(m_vertex_buffer.Count());
		cl_uint num_faces = static_cast<cl_uint>(m_face_buffer.Count());

		m_kernel_process.setArg(0, sizeof(cl_uint), &m_image_width);
		m_kernel_process.setArg(1, sizeof(cl_uint), &m_image_height);
		m_kernel_process.setArg(2, sizeof(cl_uint), &m_num_samples);
		m_kernel_process.setArg(3, sizeof(cl_uint), &num_rays);
		m_kernel_process.setArg(4, sizeof(cl_uint), &num_vertices);
		m_kernel_process.setArg(5, sizeof(cl_uint), &num_faces);
		m_kernel_process.setArg(6, m_vertex_buffer.GetBuffer());
		m_kernel_process.setArg(7, m_face_buffer.GetBuffer());
		
		if (buffer_switch) {
			m_kernel_process.setArg(8, m_ray_bufferA.GetBuffer());
			m_kernel_process.setArg(9, m_intersection_bufferA.GetBuffer());
			m_kernel_process.setArg(10, m_ray_bufferB.GetBuffer());
			m_kernel_process.setArg(11, m_intersection_bufferB.GetBuffer());
		}
		else {
			m_kernel_process.setArg(8, m_ray_bufferB.GetBuffer());
			m_kernel_process.setArg(9, m_intersection_bufferB.GetBuffer());
			m_kernel_process.setArg(10, m_ray_bufferA.GetBuffer());
			m_kernel_process.setArg(11, m_intersection_bufferA.GetBuffer());
		}
		m_kernel_process.setArg(12, m_pixel_buffer.GetBuffer());

		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process, 0, cl::NDRange(num_rays));

		if (err != CL_SUCCESS) {
			std::cout << "Error [PathTracer]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
	}

}