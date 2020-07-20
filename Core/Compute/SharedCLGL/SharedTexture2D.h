#pragma once

#include "glm.hpp"
#include "CL/cl.hpp"

#include "Compute/Compute.h"
#include "Compute/Buffer.h"
#include "Kernels/shared_defines.h"

#include "Graphics/Texture.h"

namespace LSIS::Compute {

	class SharedTexture2D : public Texture {
	public:
		SharedTexture2D(const cl::Context& context, size_t width, size_t height);
		virtual ~SharedTexture2D();

		SharedTexture2D(const SharedTexture2D& other) = delete;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		void Update(const cl::CommandQueue& queue, const cl::Buffer& buffer);
		void Update(const cl::CommandQueue& queue, const float* data);

		static void LoadKernels();

	private:
		size_t m_width;
		size_t m_height;

		uint32_t m_texture_id;
		cl_mem m_cl_shared_texture;

		//static cl::Kernel s_kernel;
		//static cl::Program s_program;
	};

}