#pragma once

#include "CL/cl.h"

#include "Platform.h"
#include "Device.h"


namespace LSIS::Compute {

	namespace Context {
		cl_context CreateContext(const std::vector<cl_context_properties>& properties, cl_device_id device_id);
	}

}