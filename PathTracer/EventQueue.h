#pragma once

#include <vector>

#include "Compute/Compute.h"

namespace LSIS {

	/// Circular buffer for events
	class EventQueue {
	public:
		
		inline EventQueue(size_t N) {
			m_data = std::vector<cl::Event>(N);
			size = N;
			for (auto& e : m_data) {
				e = cl::Event();
			}
		}

		inline virtual ~EventQueue() {
		}

		/// Returns new event, which is no longer in use
		inline cl::Event* GetNextEvent() {
			cl::Event& e = m_data[index++];
			index = index % size;
			if (e() == nullptr)
				return &e;
			CHECK(e.wait());
			return &e;
		}

		inline void WaitAll() {
			cl::WaitForEvents(m_data);
		}

	private:
		std::vector<cl::Event> m_data;
		size_t index = 0;
		size_t size;
	};

}