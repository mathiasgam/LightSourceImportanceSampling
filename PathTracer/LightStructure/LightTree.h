#pragma once

#include <vector>
#include <cinttypes>

#include "LightStructure.h"

namespace LSIS {

	class LightTree {

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
			float3* energy;
			uint* ids;
		} build_data;

		typedef struct queue_data {
			int index;
			int left;
			int right;
		} queue_data;

		typedef struct bin {
			bbox cb;
			bbox box;
			bcone cone;
			glm::vec3 energy;
			uint count;
		};

		typedef struct bin_data {
			bin* data;
			bin_data(const size_t k) { data = new bin[k]; }
			~bin_data() { delete[] data; }
		} bin_data;

		typedef struct split {
			bin bin_l;
			bin bin_r;
		} split;


		typedef struct split_data {
			split* data;
			inline split_data(const size_t k) { data = new split[k - 1]; }
			inline ~split_data() { delete[] data; }
		} split_data;

	public:
		LightTree(const SHARED::Light* lights, const size_t num_lights, size_t k = 128);
		~LightTree();

		TypedBuffer<SHARED::LightTreeNode> GetNodeBuffer();
		size_t GetNumNodes() { return m_num_nodes; }

	private:
		inline build_data allocate_build_data(size_t size);
		inline void delete_build_data(build_data data);
		inline bound initialize_build_data(build_data& data, const SHARED::Light* lights, const size_t num_lights);

		inline void calculate_splits(split_data& data_out, const bin_data& bins);
		inline int find_best_split(const split_data& splits, const float K_r, float* cost_out);

		inline int reorder_id(build_data& data, uint start, uint end, const uint32_t split_id, int k, float k0, float k1);
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

		inline void init_bins(bin_data& bin);

		inline void bin_init(bin& data);
		inline void bin_union(bin& dst, const bin& other); // store the union of the two bins in dst, to avoid creating new data
		inline void bin_update(bin& data, const bbox& cb, const bbox& box, const bcone& cone, const glm::vec3& energy);
		inline bool bin_is_empty(const bin& data);


		inline bound calc_light_bounds(const SHARED::Light* lights, const uint index, const build_data& data);

		inline uint max_axis(float3 a);
		inline glm::vec3 convert(cl_float3 vec) { return glm::vec3(vec.x, vec.y, vec.z); }
		//inline glm::vec3 convert(cl_float4 vec) { return glm::vec3(vec.x, vec.y, vec.z); }
		inline float3 convert(glm::vec3 vec) { return { vec.x, vec.y, vec.z }; }

	private:
		const size_t m_K;
		SHARED::LightTreeNode* m_nodes = nullptr;
		size_t m_num_nodes = 0;
	};

}