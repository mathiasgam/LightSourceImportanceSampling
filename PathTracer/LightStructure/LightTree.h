#pragma once

#include <vector>
#include <cinttypes>

#include "LightStructure.h"

namespace LSIS {

	class LightTree {

#define K 16

		using LightTreeNode = SHARED::LightTreeNode;
		using float3 = glm::vec3;
		using uint = uint32_t;

		typedef struct alignas(16) bbox {
			float3 pmin;
			float3 pmax;
		} bbox;

		typedef struct alignas(16) bcone {
			float3 axis;
			float theta_o;
			float theta_e;
		} bcone;

		typedef struct bound {
			bbox spatial;
			bcone oriental;
		} bound;

		typedef struct build_data {
			float3* pmin;
			float3* pmax;
			float3* centers;
			float3* axis;
			float* theta_o;
			float* theta_e;
			float* energy;
			uint* ids;
		} build_data;

		typedef struct queue_data {
			int index;
			int left;
			int right;
			bbox center_bound;
		} queue_data;

		typedef struct bin_data {
			bbox box[K];
			bcone cone[K];
			float energy[K];
			uint count[K];
		};

		typedef struct bin_accumulation_data {
			float M_A[K]; // bounding box area measure
			float M_O[K]; // bounding cone area measure
			float E[K]; // energy sum
			uint N[K]; // light count
		};

	public:
		LightTree(const SHARED::Light* lights, const size_t num_lights);
		~LightTree();

		TypedBuffer<SHARED::LightTreeNode> GetNodeBuffer();
		size_t GetNumNodes() { return m_num_nodes; }

	private:
		inline build_data allocate_build_data(size_t size);
		inline void delete_build_data(build_data data);
		inline bound initialize_build_data(build_data& data, const SHARED::Light* lights, const size_t num_lights);

		inline float accumulate_from_left(bin_accumulation_data& data_out, const bin_data& bins);
		inline float accumulate_from_right(bin_accumulation_data& data_out, const bin_data& bins);

		inline int find_best_split(const bin_accumulation_data& bin_left, const bin_accumulation_data& bin_right, const bin_data& bins);

		inline int reorder_id(build_data& data, uint start, uint end, bbox* cb_l, bbox* cb_r, const uint32_t split_bin_id, int k, float k0, float k1);
		inline int partition(build_data& data, uint start, uint end, const uint split, int k, float k0, float k1);

		inline bbox make_bbox();
		inline bbox make_bbox(const glm::vec3 p);
		inline bbox make_bbox(const glm::vec3 pmin, const glm::vec3 pmax);
		inline bcone make_bcone(const glm::vec3 axis, float theta_o, float theta_e);

		inline void swap(bcone& a, bcone& b);

		inline bbox union_bbox(bbox a, bbox b);
		inline bcone union_bcone(bcone a, bcone b);

		inline float bbox_measure(bbox b);
		inline float bcone_measure(bcone b);

		inline bound calc_light_bounds(const SHARED::Light* lights, const uint index, const build_data& data);

		inline uint max_axis(float3 a);
		inline glm::vec3 convert(cl_float3 vec) { return glm::vec3(vec.x, vec.y, vec.z); }
		//inline glm::vec3 convert(cl_float4 vec) { return glm::vec3(vec.x, vec.y, vec.z); }
		inline float3 convert(glm::vec3 vec) { return { vec.x, vec.y, vec.z }; }

	private:
		SHARED::LightTreeNode* m_nodes = nullptr;
		size_t m_num_nodes = 0;
	};

}