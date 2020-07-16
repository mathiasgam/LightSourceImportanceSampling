#include "pch.h"
#include "Buffer.h"

#include "CL/cl.h"
#include "CommandQueue.h"

namespace LSIS::Compute {

	Buffer::Buffer(cl_context context, size_t size)
	{
		cl_int err;
		m_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, size, nullptr, &err);
		m_size = size;
	}

	Buffer::~Buffer()
	{
		clReleaseMemObject(m_mem);
	}

	void Buffer::Write(cl_command_queue queue, const void* data, size_t offset, size_t size)
	{
		clEnqueueWriteBuffer(queue, m_mem, CL_TRUE, offset, size, data, 0, nullptr, nullptr);
	}

	void Buffer::Read(cl_command_queue queue, void* data, size_t offset, size_t size)
	{
		clEnqueueReadBuffer(queue, m_mem, CL_TRUE, offset, size, data, 0, nullptr, nullptr);
	}

}