#pragma once

#include "CL/cl.h"

#include "Platform.h"
#include "Device.h"
#include "Buffer.h"

namespace LSIS::Compute {

	class Context {
	public:
		Context(const cl_context_properties* propeties, cl_device_id device_id);
		virtual ~Context();

		//std::shared_ptr<Buffer> CreateBuffer(std::shared_ptr<Context> context, size_t size);

	private:
		cl_context m_context;
	};


}