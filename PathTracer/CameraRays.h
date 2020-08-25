#pragma once

#include "Compute/Compute.h"
#include "Compute/Buffer.h"

#include "Kernels/shared_defines.h"
#include "CL/cl.hpp"

namespace LSIS {

	class CameraRays {
	public:
		CameraRays(uint32_t width, uint32_t height, uint32_t samples = 1);
		virtual ~CameraRays();

		void SetResolution(uint32_t width, uint32_t height);
		void SetMultiSample(uint32_t samples);

		void GenerateRays(const TypedBuffer<SHARED::Ray>& ray_buffer, const TypedBuffer<SHARED::Sample>& sample_buffer);

		void CompileKernels();
		size_t CalculateMemory() const;

	private:
		cl_uint m_width;
		cl_uint m_height;
		cl_uint m_multi_sample;
		cl_uint m_seed = 0;

		cl::Program m_program;
		cl::Kernel m_kernel;

	};








}