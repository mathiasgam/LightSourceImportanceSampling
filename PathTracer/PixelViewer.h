#pragma once

#include "glad/glad.h"
#include "Compute/Compute.h"
#include "Compute/Buffer.h"

#include "..//Assets/Kernels/shared_defines.h"

namespace LSIS {

	class PixelViewer {
	public:
		PixelViewer(uint32_t width, uint32_t height);
		virtual ~PixelViewer();

		void SetResolution(uint32_t width, uint32_t height);

		void UpdateTexture(const TypedBuffer<SHARED::Pixel>& pixels, uint32_t width, uint32_t height);
		void UpdateTexture(const glm::vec4* pixels, uint32_t width, uint32_t height);

		void Render();

		void CompileKernels();

	private:

		cl_uint m_width, m_height;

		GLuint m_vao;
		GLuint m_vbo;
		GLuint m_texture;
		GLuint m_shader;

		cl::Program m_program;
		cl::Kernel m_kernel;

		cl_mem m_shared_texture;

	};

}