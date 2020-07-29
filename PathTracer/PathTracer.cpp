#include "pch.h"
#include "PathTracer.h"

#include "Core/Application.h"
#include "Event/Event.h"
#include "Input/KeyCodes.h"

#include "AccelerationStructure/LBVHStructure.h"

namespace LSIS {


	PathTracer::PathTracer(uint32_t width, uint32_t height)
		: m_image_width(width), m_image_height(height), m_camera(width, height), m_viewer(width, height)
	{
		PrepareCameraRays(Compute::GetContext());
		SetEventCategoryFlags(EventCategory::EventCategoryApplication | EventCategory::EventCategoryKeyboard);

		m_tracing_structure = std::make_unique<LBVHStructure>();
		CompileKernels();
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


	void PathTracer::Update(const cl::CommandQueue& queue)
	{
		// allocate memory for the image
		//std::vector<float> pixels{};
		//pixels.resize(m_image_width * m_image_height * 4);
	}

	void PathTracer::Render()
	{
		
	}

	void PathTracer::OnUpdate(float delta)
	{
		m_camera.GenerateRays(m_ray_buffer, m_intersection_buffer);

		m_tracing_structure->TraceRays(m_ray_buffer, m_intersection_buffer);

		ProcessIntersections();

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		m_viewer.Render();
	}

	bool PathTracer::OnEvent(const Event& e)
	{
		if (e.GetEventType() == EventType::KeyPressed) {
			auto key_event = (const KeyPressedEvent&)e;
			auto key = key_event.GetKey();
			if (key == KEY_B) {
				std::cout << "PT Event: " << e << std::endl;
				auto app = Application::Get();
				auto geometry = app->GetScene()->GetCollectiveMeshData();
				m_tracing_structure->Build(geometry->GetVertices(), geometry->GetNumVertices(), geometry->GetIndices(), geometry->GetNumIndices());
				return true;
			} else if (key == KEY_R){
				CompileKernels();
				m_viewer.CompileKernels();
				m_camera.CompileKernels();
				m_tracing_structure->CompileKernels();
				std::cout << "PT: Kernels Recompiled!\n";
				return true;
			}
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

		//mem_size += m_ray_buffer.Size();
		//mem_size += m_intersection_buffer.Size();
		//mem_size += m_pixel_buffer.Size();

		return mem_size;
	} 

	void PathTracer::CompileKernels()
	{
		m_program_process = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "../Assets/Kernels/process.cl");
		m_kernel_process = Compute::CreateKernel(m_program_process, "process_intersections");
	}

	void PathTracer::TraceRays()
	{
		m_camera.GenerateRays(m_ray_buffer, m_intersection_buffer);

		m_viewer.UpdateTexture(m_pixel_buffer, m_image_width, m_image_height);
		//m_tracing_structure->TraceRays(m_ray_buffer, m_intersection_buffer);
	}

	void PathTracer::PrepareCameraRays(const cl::Context& context)
	{
		size_t num_pixels = m_image_width * m_image_height;

		m_ray_buffer = TypedBuffer<SHARED::Ray>(context, CL_MEM_READ_WRITE, num_pixels);
		m_intersection_buffer = TypedBuffer<SHARED::Intersection>(context, CL_MEM_READ_WRITE, num_pixels);
		m_pixel_buffer = TypedBuffer<SHARED::Pixel>(context, CL_MEM_READ_WRITE, num_pixels);
	}

	void PathTracer::ProcessIntersections()
	{
		cl_uint num_rays = m_image_width * m_image_height;
		m_kernel_process.setArg(0, sizeof(cl_uint), &m_image_width);
		m_kernel_process.setArg(1, sizeof(cl_uint), &m_image_height);
		m_kernel_process.setArg(2, sizeof(cl_uint), &num_rays);
		m_kernel_process.setArg(3, m_ray_buffer.GetBuffer());
		m_kernel_process.setArg(4, m_intersection_buffer.GetBuffer());
		m_kernel_process.setArg(5, m_pixel_buffer.GetBuffer());

		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_process, 0, cl::NDRange(num_rays));

		if (err != CL_SUCCESS) {
			std::cout << "Error [PathTracer]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
	}

}