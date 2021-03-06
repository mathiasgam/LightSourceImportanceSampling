#pragma once

#include <memory>

#include "Core.h"
#include "Core/Layer.h"
#include "Graphics/Shader.h"

#include "Compute/SharedCLGL/SharedTexture2D.h"

#include "PixelViewer.h"
#include "BVH.h"
#include "EventQueue.h"

namespace LSIS {

	class PathTracer {
	public:
		using time = double;
		typedef struct profile_data {
			std::string parameters;
			std::string platform;
			std::string device;
			std::string host;

			std::string sampling;
			std::string attenuation;
			std::string theta_u;

			time time_render;
			time time_exstract_tri_lights;
			time time_build_lightstructure;
			time time_build_bvh;
			time time_transfer_lightstructure;
			time time_transfer_bvh;
			time time_compile_kernel_bvh;
			time time_compile_kernel_shade;

			cl_ulong time_kernel_prepare = 0;
			cl_ulong time_kernel_shade = 0;
			cl_ulong time_kernel_trace = 0;
			cl_ulong time_kernel_trace_occlusion = 0;
			cl_ulong time_kernel_process_occlusion = 0;
			cl_ulong time_kernel_process_results = 0;

			size_t num_lights;
			size_t num_primitives;
			size_t occlusion_rays;
			size_t shading_rays;
			size_t width;
			size_t height;
			size_t samples;
			size_t num_bins;
		};

		enum Method {
			naive,
			energy,
			spatial,
			lighttree
		};

		enum ClusterAttenuation {
			Center,
			Conditional,
			ConditionalMinDist,
			ZeroTest
		};

		PathTracer(uint32_t width, uint32_t height);
		virtual ~PathTracer();

		void SetImageSize(const uint32_t width, const uint32_t height);

		void Reset();
		void ResetSamples();
		void SetCameraProjection(glm::mat4 projection);

		void ProcessPass();
		void UpdateRenderTexture();

		void SetMethod(Method m);
		void SetClusterAttenuation(ClusterAttenuation atten);
		void UseFastThetaU(bool b);
		void SetUseHDRI(bool b);
		void SetNumBins(size_t num_bins);

		bool isDone() const { return m_num_samples == m_target_samples; }
		size_t GetNumSamples() const { return m_num_samples; }
		profile_data GetProfileData() const { return m_profile_data; }

		std::vector<float> GetPixelBufferData() const;

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

		void LoadSceneData();
		void LoadHDRI();

	private:
		uint32_t m_image_width, m_image_height;
		uint32_t m_num_pixels;
		uint32_t m_num_samples_per_pixel = 1;
		uint32_t m_num_concurrent_samples = m_num_pixels * m_num_samples_per_pixel;
		uint32_t m_num_rays;
		uint32_t m_num_samples = 0;
		uint32_t m_num_lights = 0;

		uint32_t m_target_samples = 10;

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

		bool use_hdri = false;

		bool use_naive = false;
		bool use_lighttree = true;
		bool use_conditional_attenuation = true;
		bool use_min_distance = true;
		bool use_orientation = false;
		bool use_fast_theta_u = true;
		bool use_zero_dist = false;

		size_t m_num_bins = 128;

		EventQueue m_event_queue = EventQueue(100);

		profile_data m_profile_data;

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
		TypedBuffer<SHARED::LightTreeNode> m_lighttree_buffer;


	};


}