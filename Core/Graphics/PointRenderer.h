#pragma once

#include "glm.hpp"

namespace LSIS {

	class PointRenderer {
	public:
		static void Init();
		static void Shutdown();

		static void BeginBatch();
		static void EndBatch();
		static void Flush();

		static void DrawPoint(const glm::vec3& position, const glm::vec3& color);

		struct Stats {
			uint32_t DrawCount = 0;
			uint32_t PointCount = 0;
		};
		
		static const Stats& GetStats();
		static void ResetStats();
	
	};

}