#include "pch.h"
#include "PathTracer.h"

#include "Core/Application.h"
#include "Event/Event.h"
#include "Input/KeyCodes.h"

#include "AccelerationStructure/LBVHStructure.h"

namespace LSIS {


	PathTracer::PathTracer(size_t width, size_t height)
		: m_image_width(width), m_image_height(height)
	{
		//m_context = context;
		m_texture = std::make_unique<SharedTexture2D>(Compute::GetContext(), width, height);
		m_window_shader = Shader::Create("../Assets/Shaders/texture.vert", "../Assets/Shaders/texture.frag");
		PrepareCameraRays(Compute::GetContext());
		SetEventCategoryFlags(EventCategory::EventCategoryApplication | EventCategory::EventCategoryKeyboard);

		m_tracing_structure = std::make_unique<LBVHStructure>();
	}

	PathTracer::~PathTracer()
	{
	}

	void PathTracer::SetImageSize(const size_t width, const size_t height)
	{
		m_image_width = width;
		m_image_height = height;

		PrepareCameraRays(Compute::GetContext());
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
		if (e.GetEventType() == EventType::KeyPressed) {
			auto key_event = (const KeyPressedEvent&)e;
			auto key = key_event.GetKey();
			if (key == KEY_B) {
				std::cout << "PT Event: " << e << std::endl;
				auto app = Application::Get();
				auto geometry = app->GetScene()->GetCollectiveMeshData();
				m_tracing_structure->Build(geometry->GetVertices(), geometry->GetNumVertices(), geometry->GetIndices(), geometry->GetNumIndices());
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