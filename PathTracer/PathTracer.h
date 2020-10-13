#pragma once

#include <memory>

#include "Core.h"
#include "Core/Layer.h"
#include "Graphics/Shader.h"

#include "Compute/SharedCLGL/SharedTexture2D.h"

#include "PixelViewer.h"
#include "BVH.h"

namespace LSIS {

	class PathTracer : public Layer {

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
		void PrepareCameraRays(const cl::Context& context);

		void BuildStructure();

		// Prepare rays from camera and result buffers for final result
		void Prepare();
		void ProcessIntersections();
		void Shade();
		void ProcessOcclusion();
		void ProcessResults();

		void ResetSamples();

		void LoadSceneData();

	private:
		uint32_t m_image_width, m_image_height;
		uint32_t m_num_pixels;
		uint32_t m_num_samples_per_pixel = 1;
		uint32_t m_num_concurrent_samples = m_num_pixels * m_num_samples_per_pixel;
		uint32_t m_num_rays;
		uint32_t m_num_samples = 0;

		SHARED::Face* m_face_data = nullptr;
		SHARED::Vertex* m_vertex_data = nullptr;
		size_t m_num_faces = 0;
		size_t m_num_vertices = 0;

		PixelViewer m_viewer;
		BVH m_bvh;

		cl::Program m_program_prepare;
		cl::Kernel m_kernel_prepare;
		
		cl::Program m_program_process;
		cl::Kernel m_kernel_process;
		cl::Kernel m_kernel_lightsample;
		cl::Kernel m_kernel_process_results;

		cl::Program m_program_shade;
		cl::Kernel m_kernel_shade;
		cl::Kernel m_kernel_shade_occlusion;

		cl::Sampler m_sampler;
		cl::Image2D m_background_texture;

		glm::mat4 m_cam_projection;

		bool ready = false;

		bool use_solid_angle = true;
		bool use_russian_roulette = false;

		// Result Buffers
		TypedBuffer<cl_int> m_state_buffer;
		TypedBuffer<cl_float3> m_result_buffer;
		TypedBuffer<cl_float3> m_throughput_buffer;
		TypedBuffer<cl_float> m_depth_buffer;
		TypedBuffer<SHARED::Pixel> m_pixel_buffer;

		// holds the source index of the compacted samples in the pass
		TypedBuffer<cl_uint> m_source_buffer;
		// holds the count of active samples left in the pass
		TypedBuffer<cl_uint> m_active_count_buffer;

		TypedBuffer<SHARED::GeometricInfo> m_geometric_buffer;
		TypedBuffer<cl_float3> m_light_contribution_buffer;

		//TypedBuffer<SHARED::Sample> m_sample_buffer;

		// Ray Buffers
		TypedBuffer<SHARED::Ray> m_ray_buffer;
		TypedBuffer<SHARED::Ray> m_occlusion_ray_buffer;
		TypedBuffer<SHARED::Intersection> m_intersection_buffer;
		TypedBuffer<cl_int> m_occlusion_buffer;

		// Geometry Buffers
		TypedBuffer<SHARED::Vertex> m_vertex_buffer;
		TypedBuffer<SHARED::Face> m_face_buffer;
		TypedBuffer<SHARED::Material> m_material_buffer;
		TypedBuffer<SHARED::Node> m_bvh_buffer;
		TypedBuffer<SHARED::AABB> m_bboxes_buffer;

		// Light Buffers
		TypedBuffer<SHARED::Light> m_lights;
		TypedBuffer<cl_float> m_cdf_power_buffer;


	};


}