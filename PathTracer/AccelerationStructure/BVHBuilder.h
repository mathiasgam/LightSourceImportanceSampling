#pragma once

#include "Kernels/shared_defines.h"
#include "Compute/Buffer.h"
#include "Compute/Compute.h"

#include "Kernel.h"

namespace LSIS {

	class BVHBuilder : Kernel {
		typedef struct morton_key {
			cl_ulong code;
			cl_uint index;
		} morton_key;
	public:
		BVHBuilder();
		virtual ~BVHBuilder();

		virtual void Compile() override;

		const std::pair<TypedBuffer<SHARED::Node>, TypedBuffer<SHARED::AABB>> Build(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces);

	private:

		void CalcPrimitiveBounds(cl_uint num_vertices, cl_uint num_faces, const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces, const TypedBuffer<cl_float3>& centers, const TypedBuffer<SHARED::AABB>& bboxes);
		void FindSceneBounds(cl_uint num_primitives, const TypedBuffer<SHARED::AABB>& bboxes, const TypedBuffer<cl_float3>& scene_bounds);
		void GenerateMortonCodes(cl_uint num_primitives, const TypedBuffer<cl_float3>& centers, const TypedBuffer<cl_float3>& scene_bounds, const TypedBuffer<morton_key>& codes);
		void SortMortonCodes(cl_uint num_primitives, const TypedBuffer<morton_key>& codes, TypedBuffer<morton_key>& codes_sorted);
		void GenerateNodes(cl_uint num_primitives, const TypedBuffer<morton_key>& codes, const TypedBuffer<SHARED::AABB>& bounds, const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& nodes_bounds);
		void Refit(const cl_uint num_primitives, const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& bboxes);

		inline uint64_t clz64(const uint64_t i) {
			return __lzcnt64(i);
		}

		inline uint64_t sigma(const uint64_t i, const uint64_t j) {
			return clz64(i ^ j);
		}

		inline glm::vec3 mad(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
			return (a * b) + c;
		}

		inline uint32_t findSplit(const morton_key* codes, const uint32_t first, const uint32_t last);
		uint32_t generate(const morton_key* codes, const SHARED::AABB* bounds, const uint32_t first, const uint32_t last, SHARED::Node* nodes, SHARED::AABB* nodes_bounds, const uint32_t idx);
		SHARED::AABB calc_bboxes(SHARED::Node* nodes, SHARED::AABB* nodes_bboxes, uint32_t idx);

	private:
		cl::Program m_program;
		cl::Kernel m_kernel_prepare;
		cl::Kernel m_kernel_scene_bounds;
		cl::Kernel m_kernel_morton_code;
		cl::Kernel m_kernel_sort;
		cl::Kernel m_kernel_hireachy;
		cl::Kernel m_kernel_refit;
	};

}