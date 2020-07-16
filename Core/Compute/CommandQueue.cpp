#include "pch.h"
#include "CommandQueue.h"

namespace LSIS::Compute {

	cl_command_queue CommandQueue::CreateCommandQueue(cl_context context, cl_device_id device)
	{
		cl_int err;
		cl_command_queue_properties props{};
		cl_command_queue queue = clCreateCommandQueue(context, device, props, &err);

		// TODO Handle errors

		return queue;
	}

}