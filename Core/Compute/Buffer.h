#pragma once

#include "Compute.h"

namespace LSIS {

	template<class T>
	class TypedBuffer {
	public:
		TypedBuffer() : m_buffer(), m_count(0) {}
		TypedBuffer(const cl::Context& context, cl_mem_flags mem_flags, size_t count) : m_buffer(context, mem_flags, sizeof(T) * count), m_count(count) {}
		virtual ~TypedBuffer(){}

		TypedBuffer(const TypedBuffer& buf) {
			m_buffer = buf.m_buffer;
			m_count = buf.m_count;
		}

		void Write(const cl::CommandQueue& queue, const T* data, size_t offset, size_t count) {
			queue.enqueueWriteBuffer(m_buffer, CL_TRUE, sizeof(T) * offset, sizeof(T) * count, std::static_pointer_cast<const void*>(data));
		}

		void Read(const cl::CommandQueue& queue, T* data, size_t offset, size_t count) const {
			queue.enqueueReadBuffer(m_buffer, CL_TRUE, sizeof(T) * offset, sizeof(T) * count, std::static_pointer_cast<void*>(data));
		}

		const cl::Buffer& GetBuffer() const { return m_buffer; }
		cl::Buffer& GetBuffer() { return m_buffer; }

		inline size_t Count() const { return m_count; }
		inline size_t Size() const { return m_count * sizeof(T); }
		inline size_t TypeSize() const { return sizeof(T); }

	private:
		cl::Buffer m_buffer;
		size_t m_count;
	};


}