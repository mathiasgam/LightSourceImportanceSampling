#include "pch.h"
#include "CommandQueue.h"

namespace LSIS::Compute {

	namespace CommandQueue {

		cl_command_queue s_queue;

		/*
		void ReadBuffer(cl_mem mem, size_t size, void* data)
		{
			clEnqueueReadBuffer(s_queue, mem, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
		}

		void WriteBuffer(cl_mem mem, size_t size, void* data)
		{
			clEnqueueWriteBuffer(s_queue, mem, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
		}
		*/

	}
}