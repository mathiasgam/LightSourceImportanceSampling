#pragma once

#include "Kernels/shared_defines.h"
#include "Mesh/Mesh.h"
#include "Compute/Buffer.h"

namespace LSIS {

	class LBVHStructure {
	public:
		LBVHStructure();
		virtual ~LBVHStructure();

		void Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices);
			   
		TypedBuffer<SHARED::Vertex> GetVertices() const { return m_buffer_vertices; }
		TypedBuffer<SHARED::Face> GetFaces() const { return m_buffer_faces; }
		TypedBuffer<SHARED::Node> GetNodes() const { return m_buffer_bvh; }
		TypedBuffer<SHARED::AABB> GetBBoxes() const { return m_buffer_bboxes; }

	private:

		struct morton_code_64_t {
			uint64_t code;
			uint32_t index;
			//morton_code_64_t(glm::vec3 pos, uint32_t index) : code(0), index(index) {}
			bool operator < (const morton_code_64_t& other) const {
				return code < other.code;
			}

			bool operator > (const morton_code_64_t& other) const {
				return code > other.code;
			}
		};

		struct AABB {
			glm::vec3 p_min;
			glm::vec3 p_max;
			AABB() : p_min(10000.0f), p_max(-10000.0f) {}
			AABB(const AABB& a, const AABB& b) : p_min(min(a.p_min, b.p_min)), p_max(max(a.p_max, b.p_max)) {}
			AABB(const glm::vec3& p) : p_min(p), p_max(p) {}
			inline void add_AABB(const AABB& aabb) { p_min = min(p_min, aabb.p_min); p_max = max(p_max, aabb.p_max); }
		};

		static uint32_t findSplit(const morton_code_64_t* codes, const uint32_t first, const uint32_t last);
		static uint32_t generate(const morton_code_64_t* codes, const uint32_t first, const uint32_t last, SHARED::Node* nodes, const uint32_t idx);
		static AABB LBVHStructure::calc_bboxes(SHARED::Node* nodes, SHARED::AABB* nodes_bboxes, const AABB* bboxes, uint32_t idx);

		void LoadGeometryBuffers(const SHARED::Vertex* vertices, size_t num_vertices, const SHARED::Face* faces, size_t num_faces);
		void LoadBVHBuffer(const SHARED::Node* nodes, const SHARED::AABB* bboxes, size_t num_nodes);

	private:

		bool isBuild = false;

		TypedBuffer<SHARED::Node> m_buffer_bvh;
		TypedBuffer<SHARED::AABB> m_buffer_bboxes;
		TypedBuffer<SHARED::Face> m_buffer_faces;
		TypedBuffer<SHARED::Vertex> m_buffer_vertices;

		size_t m_num_vertices;
		size_t m_num_faces;
		size_t m_num_nodes;

	};
}