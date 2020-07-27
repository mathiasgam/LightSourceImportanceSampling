#pragma once

#include <memory>

#include "Core.h"
#include "Core/Layer.h"
#include "Graphics/Shader.h"

#include "AccelerationStructure/AccelerationStructure.h"
#include "Compute/SharedCLGL/SharedTexture2D.h"

#include "CameraRays.h"
#include "PixelViewer.h"

namespace LSIS {

	class PathTracer : public Layer {

		//using PixelBuffer = TypedBuffer<SHARED::Pixel>;
		//using RayBuffer = TypedBuffer<SHARED::Ray>;
		//using IntersectionBuffer = TypedBuffer<SHARED::Intersection>;

	public:
		PathTracer(uint32_t width, uint32_t height);
		virtual ~PathTracer();

		void SetImageSize(const uint32_t width, const uint32_t height);

		void Update(const cl::CommandQueue& queue);
		void Render();

		virtual void OnUpdate(float delta) override;
		virtual bool OnEvent(const Event& e) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		size_t CalculateMemory() const;

	private:

		void CompileKernels();
		void TraceRays();
		void PrepareCameraRays(const cl::Context& context);

		void ProcessIntersections();

	private:
		uint32_t m_image_width, m_image_height;

		CameraRays m_camera;
		PixelViewer m_viewer;

		cl::Program m_program_process;
		cl::Kernel m_kernel_process;

		// Buffers
		TypedBuffer<SHARED::Pixel> m_pixel_buffer;
		TypedBuffer<SHARED::Ray> m_ray_buffer;
		TypedBuffer<SHARED::Intersection> m_intersection_buffer;

		Scope<AccelerationStructure> m_tracing_structure;
	};


}