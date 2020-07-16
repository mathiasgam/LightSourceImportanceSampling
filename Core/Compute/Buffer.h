#pragma once

#include <memory>

#include "CL/cl.h"
#include "Contex.h"
#include "CommandQueue.h"

namespace LSIS::Compute {

	class Buffer {
	public:
		Buffer(cl_context context, size_t size);
		virtual ~Buffer();

		void Write(cl_command_queue queue, const void* data, size_t offset, size_t size);
		void Read(cl_command_queue queue, void* data, size_t offset, size_t size);

		inline size_t GetSize() const { return m_size; };

	private:
		cl_mem m_mem;
		size_t m_size;
	};


}