#pragma once

#include <memory>

#include "CL/cl.h"
#include "Contex.h"

namespace LSIS::Compute {

	class Buffer {
		friend class Context;
	public:
		Buffer(cl_mem mem, size_t size);
		virtual ~Buffer();

		void Write(const void* data);
		void Read(void* data) const;

		inline size_t GetSize() const { return m_size; };

	private:
		cl_mem m_mem;
		size_t m_size;
	};


}