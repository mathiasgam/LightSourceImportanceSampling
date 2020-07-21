#pragma once

#include <memory>

#include "Core.h"
#include "Layer.h"
#include "Graphics/Shader.h"

#include "Compute/AccelerationStructure/AccelerationStructure.h"
#include "Compute/SharedCLGL/SharedTexture2D.h"

namespace LSIS {

	class PathTracer : public Layer {

		using PixelBuffer = Compute::TypedBuffer<SHARED::Pixel>;
		using RayBuffer = Compute::TypedBuffer<SHARED::Ray>;
		using IntersectionBuffer = Compute::TypedBuffer<SHARED::Intersection>;

	public:
		PathTracer(const cl::Context& context, size_t width, size_t height);
		virtual ~PathTracer();

		void SetImageSize(const cl::Context& context, const size_t width, const size_t height);

		void Update(const cl::CommandQueue& queue);
		void Render();

		virtual void OnUpdate(float delta) override;
		virtual bool OnEvent(const Event& e) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		size_t CalculateMemory() const;

	private:

		void TraceRays();
		void PrepareCameraRays(const cl::Context& context);

	private:
		size_t m_image_width, m_image_height;

		// Buffers
		PixelBuffer m_pixel_buffer;
		//RayBuffer m_ray_buffer;
		//IntersectionBuffer m_intersection_buffer;

		// Rendering
		Ref<Shader> m_window_shader;
		std::unique_ptr<Compute::SharedTexture2D> m_texture;

		Scope<Compute::AccelerationStructure> m_tracing_structure;

	};


}