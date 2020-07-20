#include "pch.h"
#include "SharedTexture2D.h"

namespace LSIS::Compute {

	SharedTexture2D::SharedTexture2D(const cl::Context& context, size_t width, size_t height)
		: m_width(width), m_height(height), m_texture_id(0), m_cl_shared_texture(0)
	{
		//generate the texture ID
		glGenTextures(1, &m_texture_id);
		//binnding the texture
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		//regular sampler params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//need to set GL_NEAREST
		//(not GL_NEAREST_MIPMAP_* which would cause CL_INVALID_GL_OBJECT later)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size[0], size[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_FLOAT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		cl_int status = 0;
		m_cl_shared_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, m_texture_id, &status);
		CHECK(status);

	}

	SharedTexture2D::~SharedTexture2D()
	{
		glDeleteTextures(1, &m_texture_id);
		clReleaseMemObject(m_cl_shared_texture);
	}

	void SharedTexture2D::Bind() const
	{
		glBindTexture(1, m_texture_id);
	}

	void SharedTexture2D::UnBind() const
	{
		glBindTexture(1, 0);
	}

	void SharedTexture2D::Update(const cl::CommandQueue& queue, const cl::Buffer& buffer)
	{
		clEnqueueAcquireGLObjects(queue(), 1, &m_cl_shared_texture, 0, 0, 0);

		//s_kernel.setArg(0, buffer);
		//s_kernel.setArg(1, sizeof(size_t), &m_width);
		//s_kernel.setArg(1, sizeof(size_t), &m_height);
		//clSetKernelArg(s_kernel(), 2, sizeof(m_cl_shared_texture), &m_cl_shared_texture);

		//queue.enqueueNDRangeKernel(s_kernel, 0, cl::NDRange(m_width * m_height), cl::NullRange);

		clEnqueueReleaseGLObjects(queue(), 1, &m_cl_shared_texture, 0, 0, NULL);
	}

	void SharedTexture2D::Update(const cl::CommandQueue& queue, const float* data)
	{
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_FLOAT, data);
	}

	void SharedTexture2D::LoadKernels()
	{

	}

}