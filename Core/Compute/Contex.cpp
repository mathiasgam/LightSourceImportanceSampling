#include "pch.h"
#include "Contex.h"

#include "Platform.h"

#include <iostream>
#include <vector>

namespace LSIS::Compute {

	Context::Context(const std::vector<cl_context_properties>& propeties, cl_device_id device_id)
		: m_device_id(device_id)
	{
		m_context = clCreateContext(propeties.data(), 1, &device_id, nullptr, nullptr, nullptr);
	}

	Context::~Context()
	{
		//clReleaseContext(m_context);
	}

	/*
	std::shared_ptr<Buffer> Context::CreateBuffer(std::shared_ptr<Context> context, size_t size)
	{
		cl_mem mem = clCreateBuffer(m_context, CL_MEM_READ_WRITE, size, nullptr, nullptr);
		return std::make_shared<Buffer>(mem, size);
	}
	*/
}