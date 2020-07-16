#include "pch.h"
#include "Buffer.h"

#include "CL/cl.h"
#include "CommandQueue.h"

namespace LSIS::Compute {

	Buffer::Buffer(cl_mem mem, size_t size) 
		: m_mem(mem), m_size(size)
	{
	}

	Buffer::~Buffer()
	{
	}

	void Buffer::Write(const void* data)
	{
		//CommandQueue::WriteBuffer(m_mem, m_size, data);
	}

	void Buffer::Read(void* data) const
	{
		//CommandQueue::ReadBuffer(m_mem, m_size, data);
	}

}