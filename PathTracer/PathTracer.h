#pragma once

#include <memory>

#include "Core.h"
#include "Core/Layer.h"
#include "Graphics/Shader.h"

#include "Compute/SharedCLGL/SharedTexture2D.h"

#include "CameraRays.h"
#include "PixelViewer.h"
#include "BVH.h"

namespace LSIS {

	class PathTracer : public Layer {

		//using PixelBuffer = TypedBuffer<SHARED::Pixel>;
		//using RayBuffer = TypedBuffer<SHARED::Ray>;
		//using IntersectionBuffer = TypedBuffer<SHARED::Intersection>;

	public:
		PathTracer(uint32_t width, uint32_t height);
		virtual ~PathTracer();

		void SetImageSize(const uint32_t width, const uint32_t height);

		virtual void OnUpdate(float delta) override;
		virtual bool OnEvent(const Event& e) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		size_t CalculateMemory() const;

	private:

		void CompileKernels();
		void TraceRays();
		void PrepareCameraRays(const cl::Context& context);

		void BuildStructure();

		void ProcessIntersections();

		void ResetSamples();

		void LoadLights();

	private:
		uint32_t m_image_width, m_image_height;
		uint32_t m_num_pixels;
		uint32_t m_num_rays;
		uint32_t m_num_samples = 0;

		CameraRays m_camera;
		PixelViewer m_viewer;
		BVH m_bvh;
		
		cl::Program m_program_process;
		cl::Kernel m_kernel_process;
		cl::Kernel m_kernel_lightsample;

		bool buffer_switch = true;

		// Result Buffers
		TypedBuffer<SHARED::Pixel> m_pixel_buffer;
		TypedBuffer<SHARED::Sample> m_sample_buffer;

		// Ray Buffers
		TypedBuffer<SHARED::Ray> m_ray_bufferA;
		TypedBuffer<SHARED::Ray> m_ray_bufferB;
		TypedBuffer<SHARED::Intersection> m_intersection_bufferA;
		TypedBuffer<SHARED::Intersection> m_intersection_bufferB;

		// Geometry Buffers
		TypedBuffer<SHARED::Vertex> m_vertex_buffer;
		TypedBuffer<SHARED::Face> m_face_buffer;
		TypedBuffer<SHARED::Node> m_bvh_buffer;
		TypedBuffer<SHARED::AABB> m_bboxes_buffer;

		// Light Buffers
		TypedBuffer<SHARED::Light> m_lights;


	};


}