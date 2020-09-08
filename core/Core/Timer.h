#pragma once

#include <chrono>
#include <iostream>

namespace LSIS {

	class Timer {
	public:
		Timer(const char* name) 
			: m_name(name), m_is_stopped(false)
		{
			m_start_point = std::chrono::high_resolution_clock::now();
		}

		~Timer() {
			if (!m_is_stopped) {
				Stop();
			}
		}

		void Stop() {
			auto end_point = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start_point).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();

			m_is_stopped = true;

			float duration = (end - start) * 0.001f;

			std::cout << m_name << ": " << duration << "ms" << std::endl;
		}

	private:
		const char* m_name;
		std::chrono::time_point<std::chrono::steady_clock> m_start_point;
		bool m_is_stopped;
	};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name)

}