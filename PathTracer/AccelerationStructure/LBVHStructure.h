#pragma once

#include "AccelerationStructure.h"

namespace LSIS {

	class LBVHStructure : public AccelerationStructure {
	public:
		LBVHStructure();
		virtual ~LBVHStructure();

		virtual void Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices) override;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) override;

		virtual void CompileKernels() override;

	private:

		// Defines types for the buffers
		struct Vertex {
			cl_float4 position;
			cl_float4 normal;
			cl_float4 uv;
		};

		struct Face {
			cl_uint4 index;
		};

		struct Node {
			cl_float4 min; // .w is left neighbor
			cl_float4 max; // .w is right neighbor
		};

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
			AABB() : p_min(-10000.0f), p_max(10000.0f) {}
			AABB(const AABB& a, const AABB& b) : p_min(min(a.p_min, b.p_min)), p_max(max(a.p_max, b.p_max)) {}
			AABB(const glm::vec3& p) : p_min(p), p_max(p) {}
			inline void add_AABB(const AABB& aabb) { p_min = min(p_min, aabb.p_min); p_max = max(p_max, aabb.p_max); }
		};

		static uint32_t findSplit(const morton_code_64_t* codes, const uint32_t first, const uint32_t last);
		static uint32_t generate(const morton_code_64_t* codes, const uint32_t first, const uint32_t last, Node* nodes, const uint32_t idx);
		static AABB LBVHStructure::calc_bboxes(Node* nodes, const AABB* bboxes, uint32_t idx);

		void LoadGeometryBuffers(const Vertex* vertices, size_t num_vertices, const Face* faces, size_t num_faces);
		void LoadBVHBuffer(const Node* nodes, size_t num_nodes);

	private:

		bool isBuild = false;

		cl::Program m_program;
		cl::Kernel m_kernel;

		cl::Buffer m_buffer_bvh;
		cl::Buffer m_buffer_faces;
		cl::Buffer m_buffer_vertices;

		size_t m_num_vertices;
		size_t m_num_faces;
		size_t m_num_nodes;

	};
}