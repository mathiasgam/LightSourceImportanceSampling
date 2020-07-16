#pragma once

#include <vector>

namespace LSIS {

	struct Point {
		float position[3];
		float color[3];
	};

	class PointMesh {
	public:
		PointMesh(const std::vector<Point>& points);
		virtual ~PointMesh();

		void Upload(const std::vector<Point>& data);
		void Bind();

		inline unsigned int GetNumPoints() const { return m_num_vertices; }

		std::vector<Point> Download() const;

	private:
		uint32_t m_num_vertices;
		uint32_t m_vbo;
		uint32_t m_vao;
	};

}