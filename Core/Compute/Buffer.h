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

	template<class T>
	class ObjectBuffer {
		ObjectBuffer(cl_context context, size_t count) : m_buffer(context, sizeof(T)* count) {}
		virtual ~ObjectBuffer(){}

		void Write(cl_command_queue queue, const T* data, size_t offset, size_t count) {
			m_buffer.Write(queue, std::static_pointer_cast<const void*>(data), sizeof(T) * offset, sizeof(T) * count);
		}

		void Read(cl_command_queue queue, T* datam, size_t offset, size_t count) {
			m_buffer.Read(queue, std::static_pointer_cast<void*>(data), sizeof(T) * offset, sizeof(T) * count);
		}

		inline size_t GetSize() const { return m_buffer.GetSize(); }

	private:
		Buffer m_buffer;
	};


}