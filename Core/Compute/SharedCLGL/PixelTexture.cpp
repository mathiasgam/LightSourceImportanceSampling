#include "pch.h"
#include "PixelTexture.h"

namespace LSIS::Compute {


	PixelTexture::PixelTexture(const glm::uvec2& size)
	{
		m_size = size;
		m_num_pixels = size.x * size.y;
	}

	PixelTexture::~PixelTexture()
	{
	}

	glm::uvec2 PixelTexture::GetSize() const
	{
		return m_size;
	}

	void PixelTexture::Resize(const glm::uvec2& size)
	{
		m_size = size;
		m_num_pixels = size.x * size.y;
	}

	void PixelTexture::Update(const cl::CommandQueue& queue, const TypedBuffer<SHARED::Pixel>& pixel_buffer)
	{
		clEnqueueAcquireGLObjects(queue(), 1, &m_cl_shared_texture, 0, 0, 0);
		
		m_kernel.setArg(0, pixel_buffer.GetBuffer());
		m_kernel.setArg(1, sizeof(glm::uvec2), &m_size);
		clSetKernelArg(m_kernel(), 2, sizeof(m_cl_shared_texture), &m_cl_shared_texture);

		queue.enqueueNDRangeKernel(m_kernel, 0, cl::NDRange(m_num_pixels), cl::NullRange);

		clEnqueueReleaseGLObjects(queue(), 1, &m_cl_shared_texture, 0, 0, NULL);
	}

	void PixelTexture::Render()
	{
	}

}