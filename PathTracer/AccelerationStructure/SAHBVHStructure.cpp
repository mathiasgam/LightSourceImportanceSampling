#include "pch.h"
#include "SAHBVHStructure.h"
#include "Core/Timer.h"

#include <immintrin.h>

#include <queue>

namespace LSIS {

	inline float Nth(const __m128 a, const unsigned int n) {
		CORE_ASSERT(n >= 0 && n < 4, "N out of bounds [0,3]");
		float tmp[4] alignas(16);
		_mm_store_ps(tmp, a);
		return tmp[n];
	}

	SAHBVHStructure::SAHBVHStructure(const SHARED::Vertex* vertices, const SHARED::Face* faces, const size_t num_faces)
	{
		PROFILE_SCOPE("SAH BVH Builder");
		// allocate permanent storage
		m_num_nodes = 2L * num_faces - 1L;
		m_nodes = new SHARED::Node[m_num_nodes];
		m_bboxes = new SHARED::AABB[m_num_nodes];

		build_info info = {};

		info.centers = new float4[num_faces];
		info.bounds = new float4[num_faces * 2];
		info.ids = initialize_face_ids(num_faces);

		uint32_t next_index = 0;
		info.next_index = &next_index;

		bbox cb = calc_bounds_and_centers(info.centers, info.bounds, vertices, faces, num_faces);

		auto queue = std::queue<queue_item>();
		queue.emplace(next_index++, 0, (uint32_t)num_faces, cb);

		while (!queue.empty()) {
			// fetch next node index to be processed from the queue
			queue_item args = queue.front(); 
			queue.pop();

			cb = args.cb;
			uint32_t range = args.right - args.left;
			if (range == 1) { // is leaf node
				SHARED::Node node = {};
				node.left = -1;
				node.right = info.ids[args.left];
				m_nodes[args.index] = node;
				SHARED::AABB bbox = {};
				_mm_store_ps((float*)&bbox.min, _mm_load_ps((float*)&info.bounds[node.right * 2]));
				_mm_store_ps((float*)&bbox.max, _mm_load_ps((float*)&info.bounds[node.right * 2 + 1]));
				m_bboxes[args.index] = bbox;

				//printf("Index: %d, Leaf\n", index);
				continue;
			}
			else { // is internal node
				bbox bins_bound[K];
				bbox bin_center_bounds[K];
				uint32_t bins_count[K];
				for (int i = 0; i < K; i++) {
					bins_bound[i] = make_negative_bbox(); // initialize to negtive bounds
					bins_count[i] = 0;
				}

				const int k = find_max_axis(cb.pmin, cb.pmax); // axis of the partition
				const float k0 = cb.pmin[k];
				const float k1 = (static_cast<float>(K)* (1.0f - 1e-6f)) / (cb.pmax[k] - cb.pmin[k]);

				//printf("Info: k: %d, k0: %f, k1: %f\n", k, k0, k1);
				//printf("cb_k: [%f,%f}\n", cb.pmin[k], cb.pmax[k]);

				__m128 pmin_node = _mm_set1_ps(std::numeric_limits<float>::infinity());
				__m128 pmax_node = _mm_set1_ps(-std::numeric_limits<float>::infinity());

				for (uint32_t i = args.left; i < args.right; i++) {
					const uint32_t id = info.ids[i];
					const float c_ik = info.centers[id][k];

					CORE_ASSERT(c_ik <= cb.pmax[k] && c_ik >= cb.pmin[k], "center out of bounds, failure in partition!");

					const uint32_t bin_id = static_cast<uint32_t>(k1 * (c_ik - k0));
					CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");

					const __m128 pmin = _mm_load_ps((float*)(&info.bounds[id * 2]));
					const __m128 pmax = _mm_load_ps((float*)(&info.bounds[id * 2 + 1]));

					pmin_node = _mm_min_ps(pmin_node, pmin);
					pmax_node = _mm_max_ps(pmax_node, pmax);

					const __m128 bin_pmin = _mm_load_ps((float*)&(bins_bound[bin_id].pmin));
					const __m128 bin_pmax = _mm_load_ps((float*)&(bins_bound[bin_id].pmax));

					_mm_store_ps((float*)&(bins_bound[bin_id].pmin), _mm_min_ps(pmin, bin_pmin));
					_mm_store_ps((float*)&(bins_bound[bin_id].pmax), _mm_max_ps(pmax, bin_pmax));

					bins_count[bin_id]++;
				}

				SHARED::Node node = {};
				node.left = next_index++;
				node.right = next_index++;
				m_nodes[args.index] = node;

				SHARED::AABB bbox_node = {};
				_mm_store_ps((float*)&bbox_node.min, pmin_node);
				_mm_store_ps((float*)&bbox_node.max, pmax_node);
				m_bboxes[args.index] = bbox_node;

				float A_l[K];
				uint32_t N_l[K];
				accumulate_from_left(A_l, N_l, bins_bound, bins_count);

				float A_r[K];
				uint32_t N_r[K];
				accumulate_from_right(A_r, N_r, bins_bound, bins_count);

				const uint32_t best_split_bin_id = find_optimal_split(A_l, A_r, N_l, N_r);

				bbox cb_l, cb_r;
				uint32_t middle = reorder_ids(info, args.left, args.right, &cb_l, &cb_r, best_split_bin_id, k, k0, k1);

				queue.emplace(node.left, args.left, middle, cb_l);
				queue.emplace(node.right, middle, args.right, cb_r);

			}
		}

		//uint32_t last = build_recursive(info, 0, num_faces, cb);
		//CORE_ASSERT(last == num_nodes, "generated and allocated nodes not matching!");

		// clean allocated memory
		delete[] info.centers;
		delete[] info.bounds;
		delete[] info.ids;
	}

	SAHBVHStructure::~SAHBVHStructure()
	{
	}

	TypedBuffer<SHARED::AABB> SAHBVHStructure::GetBoundsBuffer()
	{
		TypedBuffer<SHARED::AABB> buffer = TypedBuffer<SHARED::AABB>(Compute::GetContext(), CL_MEM_READ_ONLY, m_num_nodes);
		Compute::GetCommandQueue().enqueueWriteBuffer(buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * m_num_nodes, m_bboxes);
		return buffer;
	}

	TypedBuffer<SHARED::Node> SAHBVHStructure::GetNodesBuffer()
	{
		TypedBuffer<SHARED::Node> buffer = TypedBuffer<SHARED::Node>(Compute::GetContext(), CL_MEM_READ_ONLY, m_num_nodes);
		Compute::GetCommandQueue().enqueueWriteBuffer(buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Node) * m_num_nodes, m_nodes);
		return buffer;
	}

	SAHBVHStructure::bbox SAHBVHStructure::calc_bounds_and_centers(float4* centers, float4* bounds, const SHARED::Vertex* vertices, const SHARED::Face* faces, const size_t num_faces)
	{
		__m128 cb_min = _mm_set1_ps(std::numeric_limits<float>::infinity());
		__m128 cb_max = _mm_set1_ps(-std::numeric_limits<float>::infinity());

		const SHARED::Face* end = faces + num_faces;
		float4* center_it = centers;
		float4* bound_it = bounds;
		for (const SHARED::Face* face = faces; face < end; face++) {
			const cl_uint4 index = face->index;
			const __m128 p0 = _mm_load_ps((const float*)(&(vertices[index.x].position)));
			const __m128 p1 = _mm_load_ps((const float*)(&(vertices[index.y].position)));
			const __m128 p2 = _mm_load_ps((const float*)(&(vertices[index.z].position)));

			const __m128 pmin = _mm_min_ps(_mm_min_ps(p0, p1), p2);
			const __m128 pmax = _mm_max_ps(_mm_max_ps(p0, p1), p2);

			const __m128 center = _mm_mul_ps(_mm_add_ps(pmin, pmax), _mm_set_ps1(0.5f));

			cb_min = _mm_min_ps(cb_min, center);
			cb_max = _mm_max_ps(cb_max, center);

			_mm_store_ps((float*)bound_it++, pmin);
			_mm_store_ps((float*)bound_it++, pmax);
			_mm_store_ps((float*)center_it++, center);
		}
		bbox cb = {};
		_mm_store_ps((float*)&(cb.pmin), cb_min);
		_mm_store_ps((float*)&(cb.pmax), cb_max);
		return cb;
	}

	inline uint32_t* SAHBVHStructure::initialize_face_ids(size_t num_faces)
	{
		uint32_t* face_ids = new uint32_t[num_faces];
		for (uint32_t i = 0; i < num_faces; i++) {
			face_ids[i] = i;
		}
		return face_ids;
	}

	inline uint32_t SAHBVHStructure::build_recursive(build_info info, uint32_t begin, uint32_t end, const bbox cb)
	{
		bbox cb_l, cb_r;
		uint32_t middle;
		const uint32_t index = (*info.next_index)++;
		{
			int num = end - begin;
			//printf("Build Recurse: Num: %d\n", num);
			if (num == 1) {
				SHARED::Node node = {};
				node.left = -1;
				node.right = info.ids[begin];
				m_nodes[index] = node;
				SHARED::AABB bbox = {};
				_mm_store_ps((float*)&bbox.min, _mm_load_ps((float*)&info.bounds[node.right * 2]));
				_mm_store_ps((float*)&bbox.max, _mm_load_ps((float*)&info.bounds[node.right * 2 + 1]));
				m_bboxes[index] = bbox;

				//printf("Index: %d, Leaf\n", index);
				return index;
			}
			//printf("Index: %d, Internal\n", index);
			//printf("cb: pmin: %f,%f,%f, pmax: %f,%f,%f\n", cb.pmin.x, cb.pmin.y, cb.pmin.z, cb.pmax.x, cb.pmax.y, cb.pmax.z);

			bbox bins_bound[K];
			bbox bin_center_bounds[K];
			uint32_t bins_count[K];
			for (int i = 0; i < K; i++) {
				bins_bound[i] = make_negative_bbox(); // initialize to negtive bounds
				bins_count[i] = 0;
			}

			const int k = find_max_axis(cb.pmin, cb.pmax); // axis of the partition
			const float k0 = cb.pmin[k];
			const float k1 = (static_cast<float>(K)* (1.0f - 1e-6f)) / (cb.pmax[k] - cb.pmin[k]);

			//printf("Info: k: %d, k0: %f, k1: %f\n", k, k0, k1);
			//printf("cb_k: [%f,%f}\n", cb.pmin[k], cb.pmax[k]);

			__m128 pmin_node = _mm_set1_ps(std::numeric_limits<float>::infinity());
			__m128 pmax_node = _mm_set1_ps(-std::numeric_limits<float>::infinity());

			for (uint32_t i = begin; i < end; i++) {
				const uint32_t id = info.ids[i];
				const float c_ik = info.centers[id][k];

				CORE_ASSERT(c_ik <= cb.pmax[k] && c_ik >= cb.pmin[k], "center out of bounds, failure in partition!");

				const uint32_t bin_id = static_cast<uint32_t>(k1 * (c_ik - k0));
				CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");

				const __m128 pmin = _mm_load_ps((float*)(&info.bounds[id * 2]));
				const __m128 pmax = _mm_load_ps((float*)(&info.bounds[id * 2 + 1]));

				pmin_node = _mm_min_ps(pmin_node, pmin);
				pmax_node = _mm_max_ps(pmax_node, pmax);

				const __m128 bin_pmin = _mm_load_ps((float*)&(bins_bound[bin_id].pmin));
				const __m128 bin_pmax = _mm_load_ps((float*)&(bins_bound[bin_id].pmax));

				_mm_store_ps((float*)&(bins_bound[bin_id].pmin), _mm_min_ps(pmin, bin_pmin));
				_mm_store_ps((float*)&(bins_bound[bin_id].pmax), _mm_max_ps(pmax, bin_pmax));

				bins_count[bin_id]++;
			}

			SHARED::AABB bbox_node = {};
			_mm_store_ps((float*)&bbox_node.min, pmin_node);
			_mm_store_ps((float*)&bbox_node.max, pmax_node);
			m_bboxes[index] = bbox_node;

			float A_l[K];
			uint32_t N_l[K];
			accumulate_from_left(A_l, N_l, bins_bound, bins_count);

			float A_r[K];
			uint32_t N_r[K];
			accumulate_from_right(A_r, N_r, bins_bound, bins_count);

			const uint32_t best_split_bin_id = find_optimal_split(A_l, A_r, N_l, N_r);

			middle = reorder_ids(info, begin, end, &cb_l, &cb_r, best_split_bin_id, k, k0, k1);
		}

		//printf("Range: left: %zd, middle: %zd, right: %zd\n", (size_t)begin, (size_t)middle, (size_t)end);
		SHARED::Node node = {};
		node.left = build_recursive(info, begin, middle, cb_l);
		node.right = build_recursive(info, middle, end, cb_r);

		m_nodes[index] = node;

		return index;
	}

	inline uint32_t SAHBVHStructure::find_max_axis(float4 pmin, float4 pmax)
	{
		const float x = pmax.x - pmin.x;
		const float y = pmax.y - pmin.y;
		const float z = pmax.z - pmin.z;
		return (x > y) ? ((x > z) ? 0 : 2) : ((y > z) ? 1 : 2);
	}

	inline float SAHBVHStructure::calc_area(const __m128 diagonal)
	{
		// calculate area of the bounding volume
		const __m128 xxyw = _mm_shuffle_ps(diagonal, diagonal, _MM_SHUFFLE(3, 1, 0, 0));
		const __m128 yzzw = _mm_shuffle_ps(diagonal, diagonal, _MM_SHUFFLE(3, 2, 2, 1));
		const __m128 mul = _mm_mul_ps(xxyw, yzzw);
		const __m128 mul2 = _mm_mul_ps(mul, _mm_set_ps(2.0f, 2.0f, 2.0f, 0.0f));
		__m128 sum = _mm_hadd_ps(mul2, mul2);
		sum = _mm_hadd_ps(mul2, mul2);
		return _mm_cvtss_f32(sum);
	}

	inline void SAHBVHStructure::accumulate_from_left(float* A_l, uint32_t* N_l, const bbox* bin_bounds, const uint32_t* bin_counts)
	{
		// accumulate counts
		uint32_t count = 0;
		for (int i = 0; i < K; i++) {
			count += bin_counts[i];
			N_l[i] = count;
		}

		// Accumulate bounds and calculate areas
		__m128 pmin = _mm_set1_ps(std::numeric_limits<float>::infinity());
		__m128 pmax = _mm_set1_ps(-std::numeric_limits<float>::infinity());
		for (int i = 0; i < K; i++) {
			pmin = _mm_min_ps(pmin, _mm_load_ps((float*)&(bin_bounds[i].pmin)));
			pmax = _mm_max_ps(pmax, _mm_load_ps((float*)&(bin_bounds[i].pmax)));

			const __m128 diagonal = _mm_sub_ps(pmax, pmin);
			A_l[i] = calc_area(diagonal);
		}
	}

	inline void SAHBVHStructure::accumulate_from_right(float* A_r, uint32_t* N_r, const bbox* bin_bounds, const uint32_t* bin_counts)
	{
		// accumulate counts
		uint32_t count = 0;
		for (int i = K - 1L; i >= 0; i--) {
			count += bin_counts[i];
			N_r[i] = count;
		}

		// Accumulate bounds and calculate areas
		__m128 pmin = _mm_set1_ps(std::numeric_limits<float>::infinity());
		__m128 pmax = _mm_set1_ps(-std::numeric_limits<float>::infinity());
		for (int i = K - 1L; i >= 0; i--) {
			pmin = _mm_min_ps(pmin, _mm_load_ps((float*)&(bin_bounds[i].pmin)));
			pmax = _mm_max_ps(pmax, _mm_load_ps((float*)&(bin_bounds[i].pmax)));

			const __m128 diagonal = _mm_sub_ps(pmax, pmin);
			A_r[i] = calc_area(diagonal);
		}
	}

	inline uint32_t SAHBVHStructure::find_optimal_split(float* A_l, float* A_r, uint32_t* N_l, uint32_t* N_r)
	{
		// initialize best score and index to first bin
		float cost_best = A_l[0] * N_l[0] + A_r[0] * N_r[0];
		uint32_t i_best = 0;

		// iterate through the rest of the bins and save everytime a better score is found
		for (uint32_t i = 1; i < K-1; i++) {
			const float cost = A_l[i] * N_l[i] + A_r[i+1] * N_r[i+1];
			if (cost < cost_best) {
				cost_best = cost;
				i_best = i;
			}
		}

		return i_best;
	}

	inline uint32_t SAHBVHStructure::reorder_ids(build_info info, uint32_t begin, uint32_t end, bbox* cb_l, bbox* cb_r, const uint32_t split_bin_id, int k, float k0, float k1)
	{
		uint32_t left = begin;
		uint32_t right = end - 1;

		__m128 pmin_l = _mm_set1_ps(std::numeric_limits<float>::infinity());
		__m128 pmin_r = _mm_set1_ps(std::numeric_limits<float>::infinity());
		__m128 pmax_l = _mm_set1_ps(-std::numeric_limits<float>::infinity());
		__m128 pmax_r = _mm_set1_ps(-std::numeric_limits<float>::infinity());

		while (left < right) {
			const uint32_t id_l = info.ids[left];
			const __m128 c_il = _mm_load_ps((float*)&info.centers[id_l]);
			const uint32_t bin_id_l = static_cast<uint32_t>(k1 * (Nth(c_il, k) - k0));

			if (bin_id_l > split_bin_id) {
				while (left < right) {
					const uint32_t id_r = info.ids[right];
					const __m128 c_ir = _mm_load_ps((float*)&info.centers[id_r]);
					const uint32_t bin_id_r = static_cast<uint32_t>(k1 * (Nth(c_ir, k) - k0));

					if (bin_id_r <= split_bin_id) {
						info.ids[right] = id_l;
						info.ids[left] = id_r;
						break;
					}
					else {
						// add center to right bound
						const __m128 c_ir_sse = _mm_load_ps((float*)&c_ir);
						pmin_r = _mm_min_ps(pmin_r, c_ir_sse);
						pmax_r = _mm_max_ps(pmax_r, c_ir_sse);
						right--;
					}
				}
			}
			else {
				// add center to left bound
				const __m128 c_il_sse = _mm_load_ps((float*)&c_il);
				pmin_l = _mm_min_ps(pmin_l, c_il_sse);
				pmax_l = _mm_max_ps(pmax_l, c_il_sse);
				left++;
			}
		}

		const __m128 c = _mm_load_ps((float*)&info.centers[info.ids[left]]);
		const uint32_t bin_id = static_cast<uint32_t>(k1 * (Nth(c, k) - k0));
		if (bin_id > split_bin_id) {
			pmin_r = _mm_min_ps(c, pmin_r);
			pmax_r = _mm_max_ps(c, pmax_r);
		}
		else {
			pmin_l = _mm_min_ps(c, pmin_l);
			pmax_l = _mm_max_ps(c, pmax_l);
		}


		_mm_store_ps((float*)&((*cb_l).pmin), pmin_l);
		_mm_store_ps((float*)&((*cb_l).pmax), pmax_l);
		_mm_store_ps((float*)&((*cb_r).pmin), pmin_r);
		_mm_store_ps((float*)&((*cb_r).pmax), pmax_r);

#ifdef DEBUG
		// check left partition
		const int k_l = find_max_axis(cb_l->pmin, cb_l->pmax); // axis of the partition
		const float k0_l = cb_l->pmin[k_l];
		const float k1_l = (static_cast<float>(K)* (1.0f - 1e-6f)) / (cb_l->pmax[k_l] - cb_l->pmin[k_l]);
		for (int i = begin; i < left; i++) {
			const float4 c_i = info.centers[info.ids[i]];
			const uint32_t bin_id = static_cast<uint32_t>(k1_l * (c_i[k_l] - k0_l));
			CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");
			//CORE_ASSERT(bin_id <= split_bin_id, "WRONG PARTITION!");
		}

		// check right partition
		const int k_r = find_max_axis(cb_r->pmin, cb_r->pmax); // axis of the partition
		const float k0_r = cb_r->pmin[k_r];
		const float k1_r = (static_cast<float>(K)* (1.0f - 1e-6f)) / (cb_r->pmax[k_r] - cb_r->pmin[k_r]);
		for (int i = left; i < end; i++) {
			const float4 c_i = info.centers[info.ids[i]];
			const uint32_t bin_id = static_cast<uint32_t>(k1_r * (c_i[k_r] - k0_r));
			CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");
			//CORE_ASSERT(bin_id > split_bin_id, "WRONG PARTITION!");
		}
#endif // DEBUG

		CORE_ASSERT(left == right, "left and right has to match!");

		return left;
	}

	SAHBVHStructure::queue_item::queue_item(uint32_t index, uint32_t left, uint32_t right, bbox cb)
		:index(index), left(left), right(right), cb(cb)
	{
	}

}