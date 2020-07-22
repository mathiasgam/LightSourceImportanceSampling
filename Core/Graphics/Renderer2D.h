#pragma once

#include "glm.hpp"

namespace LSIS {

	class LineRenderer {
	public:
		static void Init();
		static void Shutdown();

		static void BeginBatch();
		static void EndBatch();
		static void Flush();

		static void DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color);
		static void DrawLines(const glm::vec3* points, size_t n, const glm::vec3& color);

		struct Stats {
			uint32_t DrawCount = 0;
			uint32_t LineCount = 0;
		};

		static const Stats& GetStats();
		static void ResetStats();

	};

}