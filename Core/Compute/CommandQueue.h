#pragma once

#include "CL/cl.h"
#include "Contex.h"
#include "Buffer.h"

namespace LSIS::Compute {

	namespace CommandQueue {
		cl_command_queue CreateCommandQueue(cl_context context, cl_device_id device);
	}

}