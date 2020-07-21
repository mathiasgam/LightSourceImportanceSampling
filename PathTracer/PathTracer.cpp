#include "pch.h"
#include "PathTracer.h"

namespace LSIS {


	PathTracer::PathTracer(const cl::Context& context, size_t width, size_t height)
		: m_image_width(width), m_image_height(height)
	{
		//m_context = context;
		m_texture = std::make_unique<Compute::SharedTexture2D>(context, width, height);
		m_window_shader = Shader::Create("../Assets/Kernels/texture.vert", "../Assets/Kernels/texture.frag");
		PrepareCameraRays(context);
		SetEventCategoryFlags(EventCategory::EventCategoryApplication);
	}

	PathTracer::~PathTracer()
	{
	}

	void PathTracer::SetImageSize(const cl::Context& context, const size_t width, const size_t height)
	{
		m_image_width = width;
		m_image_height = height;

		PrepareCameraRays(context);
	}


	void PathTracer::Update(const cl::CommandQueue& queue)
	{
		// allocate memory for the image
		std::vector<float> pixels{};
		pixels.resize(m_image_width * m_image_height * 4);

		size_t index = 0;
		for (size_t x = 0; x < m_image_width; x++) {
			for (size_t y = 0; y < m_image_height; y++) {
				pixels[index++] = 1.0f;
				pixels[index++] = 0.0f;
				pixels[index++] = 0.0f;
				pixels[index++] = 1.0f;
			}
		}
		m_texture->Update(queue, pixels.data());
	}

	void PathTracer::Render()
	{
	}

	void PathTracer::OnUpdate(float delta)
	{
	}

	bool PathTracer::OnEvent(const Event& e)
	{
		std::cout << "PT Event: " << e << std::endl;
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
		mem_size += m_pixel_buffer.Size();

		return mem_size;
	} 

	void PathTracer::TraceRays()
	{
		//m_tracing_structure->TraceRays(m_ray_buffer, m_intersection_buffer);
	}

	void PathTracer::PrepareCameraRays(const cl::Context& context)
	{
		size_t num_pixels = m_image_width * m_image_height;

		//m_ray_buffer = RayBuffer(m_context->GetContext(), CL_MEM_READ_WRITE, num_pixels);
		//m_intersection_buffer = IntersectionBuffer(m_context->GetContext(), CL_MEM_READ_WRITE, num_pixels);
		m_pixel_buffer = PixelBuffer(context, CL_MEM_READ_WRITE, num_pixels);
	}

}