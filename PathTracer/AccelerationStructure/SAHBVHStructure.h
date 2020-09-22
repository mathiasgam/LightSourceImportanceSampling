#pragma once

#include "Kernels/shared_defines.h"
#include "Mesh/Mesh.h"
#include "Compute/Buffer.h"

namespace LSIS {

#define K 16


	class SAHBVHStructure
	{
	private:
		typedef struct alignas(16) float4 {
			float x;
			float y;
			float z;
			float w;

			float& operator[](int i) { return *(&x+i); }
			const float& operator[](int i) const { return *(&x + i); }
			float* operator*() { return &x; }
			const float* operator*() const { return &x; }
		} float4;

		float4 make_float4(float f) {
			return { f,f,f,f };
		}

		float4 make_float4(float x, float y, float z, float w) {
			return { x,y,z,w };
		}

		typedef struct alignas(32) bbox {
			float4 pmin;
			float4 pmax;
		} bbox;

		bbox make_negative_bbox() {
			return { make_float4(std::numeric_limits<float>::infinity()), make_float4(-std::numeric_limits<float>::infinity()) };
		}

		typedef struct build_info {
			float4* centers;
			float4* bounds;
			uint32_t* ids;
			uint32_t* next_index;
		} build_info;

		typedef struct queue_item {
			uint32_t index;
			uint32_t left;
			uint32_t right;
			bbox cb;
			queue_item(uint32_t index, uint32_t left, uint32_t right, bbox cb);
			queue_item() = delete;
		} queue_item;


	public:
		SAHBVHStructure(const SHARED::Vertex* vertices, const SHARED::Face* faces, const size_t num_faces);
		~SAHBVHStructure();

		TypedBuffer<SHARED::AABB> GetBoundsBuffer();
		TypedBuffer<SHARED::Node> GetNodesBuffer();

	private:

		// Iterate over all faces and calculate the AABBs and centroids. returns the bounding volume for all face centroids
		inline bbox calc_bounds_and_centers(float4* centers, float4* bounds, const SHARED::Vertex* vertices, const SHARED::Face* faces, const size_t num_faces);
		// return an array of size 'num_faces', initialized with 0,1,2...num_faces-1
		inline uint32_t* initialize_face_ids(size_t num_faces);

		inline uint32_t build_recursive(build_info info, uint32_t begin, uint32_t end, const bbox cb);
		inline uint32_t find_max_axis(float4 pmin, float4 pmax);
		inline float calc_area(const __m128 diagonal);
		inline void accumulate_from_left(float* A_l, uint32_t* N_l, const bbox* bin_bounds, const uint32_t* bin_counts);
		inline void accumulate_from_right(float* A_r, uint32_t* N_r, const bbox* bin_bounds, const uint32_t* bin_counts);
		inline uint32_t find_optimal_split(float* A_l, float* A_r, uint32_t* N_l, uint32_t* N_r);
		inline uint32_t reorder_ids(build_info info, uint32_t begin, uint32_t end, bbox* cb_l, bbox* cb_r, const uint32_t split_bin_id, int k, float k0, float k1);

		//int BuildRecursive(Bound* bounds, glm::vec3* centers, int index, int start, int end, Bound partition_bound);
		//float Cost(Bound A_l, int N_l, Bound A_r, int N_r);

	private:
		SHARED::Node* m_nodes;
		SHARED::AABB* m_bboxes;
		size_t m_num_nodes;

	};

}