#pragma once

#include "glm.hpp"
#include "CL/cl.hpp"

#include "Compute/Compute.h"
#include "Compute/Buffer.h"
#include "Kernels/shared_defines.h"

namespace LSIS::Compute {

	class PixelTexture {
	public:
		PixelTexture(const glm::uvec2& size);
		virtual ~PixelTexture();

		glm::uvec2 GetSize() const;

		void Resize(const glm::uvec2& size);
		void Update(const cl::CommandQueue& queue, const TypedBuffer<SHARED::Pixel>& pixel_buffer);

		void Render();

	private:
		cl::Program m_program;
		cl::Kernel m_kernel;

		unsigned int m_gl_texture;
		cl_mem m_cl_shared_texture;

		glm::uvec2 m_size;
		uint32_t m_num_pixels;
	};

}