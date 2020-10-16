#include "pch.h"
#include "LightTree.h"

#include "Core/Timer.h"

#include "gtc/constants.hpp"
#include "gtx/rotate_vector.hpp"

namespace LSIS {

	LightTree::LightTree(const SHARED::Light* lights, const size_t num_lights)
	{
		PROFILE_SCOPE("LightTree Construction");
		if (num_lights == 0) {
			m_num_nodes = 0;
			return;
		}

		// Allocate node buffer
		m_num_nodes = num_lights * 2L - 1L;
		m_nodes = new SHARED::LightTreeNode[m_num_nodes];

		// Allocate build data
		build_data data = allocate_build_data(num_lights);
		int next_index = 0;

		// Initialize build data and calculate total bound for all lights
		bound lights_bound = initialize_build_data(data, lights, num_lights);

		// Initialize queue and push the root node
		auto queue = std::queue<queue_data>();
		queue.push({ next_index++, 0, (int)num_lights, lights_bound.spatial });

		bin_data bins[3];
		bin_accumulation_data bin_left = {};
		bin_accumulation_data bin_right = {};
		split_data splits[3];

		// Do recursive function using queue to avoid stack overflow with many lights
		while (!queue.empty()) {
			// Fetch next iteration data from the queue and pop the element
			const auto [index, left, right, cb] = queue.front(); queue.pop();
			//printf("index: %d, left: %d, right: %d\n", index, left, right);
			CORE_ASSERT(index >= 0 && index < m_num_nodes, "index out of bounds!");
			CORE_ASSERT(left >= 0 && left < num_lights, "left is out of bounds!");
			CORE_ASSERT(right > 0 && right <= num_lights, "right is out of bounds!");

			const int range = right - left;
			CORE_ASSERT(range > 0 && range <= num_lights, "Range is out of bounds");
			if (range == 1) { // Is 
				uint id = data.ids[left];

				const float3 pmin = data.pmin[id];
				const float3 pmax = data.pmax[id];
				const float3 axis = data.axis[id];
				const float energy = data.energy[id];
				const float theta_o = data.theta_o[id];
				const float theta_e = data.theta_e[id];

				m_nodes[index] = SHARED::make_light_tree_node(pmin, pmax, axis, glm::vec3(energy), theta_o, theta_e, -1, left);
			}
			else if (range == 2) {

				const int index_left = next_index++;
				const int index_right = next_index++;
				const int middle = left + 1;

				uint id_l = data.ids[left];
				uint id_r = data.ids[left + 1];

				bbox box_left = make_bbox(data.pmin[id_l], data.pmax[id_l]);
				bbox box_right = make_bbox(data.pmin[id_r], data.pmax[id_r]);

				bcone cone_left = make_bcone(data.axis[id_l], data.theta_o[id_l], data.theta_e[id_l]);
				bcone cone_right = make_bcone(data.axis[id_r], data.theta_o[id_r], data.theta_e[id_r]);

				glm::vec3 energy_left = glm::vec3(data.energy[id_l]);
				glm::vec3 energy_right = glm::vec3(data.energy[id_r]);

				bbox box = union_bbox(box_left, box_right);
				bcone cone = union_bcone(cone_left, cone_right);
				glm::vec3 energy = energy_left + energy_right;

				// create parent node
				m_nodes[index] = SHARED::make_light_tree_node(box.pmin, box.pmax, cone.axis, energy, cone.theta_o, cone.theta_e, index_left, index_right);

				// and create the two child nodes. No need for queing
				m_nodes[index_left] = SHARED::make_light_tree_node(box_left.pmin, box_left.pmax, cone_left.axis, energy_left, cone_left.theta_o, cone_left.theta_e, -1, left);
				m_nodes[index_right] = SHARED::make_light_tree_node(box_right.pmin, box_right.pmax, cone_right.axis, energy_right, cone_right.theta_o, cone_right.theta_e, -1, left + 1);
			}
			else { // Is Internal

				// set the counts to zero to indicate the bins are empty
				init_bins(bins[0]);
				init_bins(bins[1]);
				init_bins(bins[2]);

				const glm::vec3 diagonal = cb.pmax - cb.pmin;
				const uint k = max_axis(diagonal);
				const glm::vec3 k0 = cb.pmin;
				const glm::vec3 k1 = (static_cast<float>(K)* (1.0f - 1e-6f)) / (diagonal);

				// Calculate bins
				for (int i = left; i < right; i++) {
					const uint id = data.ids[i];
					const glm::vec3 c_i = data.centers[id];
					if (cb.pmin[k] > c_i[k] || cb.pmax[k] < c_i[k]) {
						printf("ERROR: k: %d, pmin: %f, pmax: %f, c_ik: %f\n", k, cb.pmin[k], cb.pmax[k], c_i[k]);
					}
					const glm::ivec3 bin_id = static_cast<glm::ivec3>(k1[k] * (c_i[k] - k0[k]));
					CORE_ASSERT(bin_id[k] >= 0 && bin_id[k] < K, "Bin ID out of bounds!");

					const bbox cb_i = make_bbox(c_i);
					const bbox box_i = make_bbox(data.pmin[id], data.pmax[id]);
					const bcone cone_i = make_bcone(data.axis[id], data.theta_o[id], data.theta_e[id]);
					const glm::vec3 e_i = glm::vec3(data.energy[id]);

					bin_update(bins[0].data[bin_id[k]], cb_i, box_i, cone_i, e_i);
				}

				calculate_splits(splits[0], bins[0]);

				const float K_r = glm::max(glm::max(diagonal.x,diagonal.y), diagonal.z) / diagonal[k];
				const int best_split = find_best_split(splits[0], K_r);

				const int middle = reorder_id(data, left, right, best_split, k, k0[k], k1[k]);

				//const int middle = left + (range / 2);
				const int index_left = next_index++;
				const int index_right = next_index++;

				const bbox total_box = splits[0].data[0].bin_r.box;
				const bcone total_cone = splits[0].data[0].bin_r.cone;
				const glm::vec3 total_energy = splits[0].data[0].bin_r.energy;

				const float3 pmin = total_box.pmin;
				const float3 pmax = total_box.pmax;
				const float3 axis = total_cone.axis;
				const float theta_o = total_cone.theta_o;
				const float theta_e = total_cone.theta_e;

				m_nodes[index] = SHARED::make_light_tree_node(pmin, pmax, axis, total_energy, theta_o, theta_e, index_left, index_right);

				//printf("index: %d, left: %d, middle: %d, right: %d\n", index, left, middle, right);

				CORE_ASSERT(left != middle && middle != right, "only non zero ranges are allowed!");

				queue.push({ index_left, left, middle, splits[0].data[best_split].bin_l.cb });
				queue.push({ index_right, middle, right, splits[0].data[best_split].bin_r.cb });
			}
		}

		// Delete build data
		delete_build_data(data);
	}
	LightTree::~LightTree()
	{
		if (m_nodes)
			delete[] m_nodes;
	}
	TypedBuffer<SHARED::LightTreeNode> LightTree::GetNodeBuffer()
	{
		// Return empty buffer if no nodes are available
		if (m_num_nodes == 0)
			return TypedBuffer<SHARED::LightTreeNode>();

		// Fetch command queue
		const auto queue = Compute::GetCommandQueue();

		// create buffer and copy node data to the GPU
		TypedBuffer<SHARED::LightTreeNode> buffer = TypedBuffer<SHARED::LightTreeNode>(Compute::GetContext(), CL_MEM_READ_ONLY, m_num_nodes);
		CHECK(queue.enqueueWriteBuffer(buffer.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::LightTreeNode) * m_num_nodes, (void*)m_nodes));

		return buffer;
	}
	inline LightTree::build_data LightTree::allocate_build_data(size_t size)
	{
		build_data data = {};
		data.pmin = new float3[size];
		data.pmax = new float3[size];
		data.centers = new float3[size];
		data.axis = new float3[size];
		data.theta_o = new float[size];
		data.theta_e = new float[size];
		data.energy = new float[size];
		data.ids = new uint[size];
		return data;
	}
	inline void LightTree::delete_build_data(build_data data)
	{
		delete[] data.pmin;
		delete[] data.pmax;
		delete[] data.centers;
		delete[] data.axis;
		delete[] data.theta_o;
		delete[] data.theta_e;
		delete[] data.energy;
		delete[] data.ids;
	}
	inline LightTree::bound LightTree::initialize_build_data(build_data& data, const SHARED::Light* lights, const size_t num_lights)
	{
		// Allocate global bound struct
		bound cb = calc_light_bounds(lights, 0, data);

		// Handle rest of the lights
		for (int i = 1; i < num_lights; i++) {
			bound cb_i = calc_light_bounds(lights, i, data);

			// Update global bounds
			cb.spatial = union_bbox(cb.spatial, cb_i.spatial);
			cb.oriental = union_bcone(cb.oriental, cb_i.oriental);
		}

		// Return global bound
		return cb;
	}
	inline float LightTree::accumulate_from_left(bin_accumulation_data& data_out, const bin_data& bins)
	{
		// Initialize accumulative data
		bin accumulation = {};
		bin_init(accumulation);

		int N = 0;
		glm::vec3 E = glm::vec3(std::numeric_limits<float>::infinity());
		float M_A = std::numeric_limits<float>::infinity();
		float M_O = std::numeric_limits<float>::infinity();

		// Handle rest of the bins usually
		for (int i = 0; i < K; i++) {
			const bin& other = bins.data[i];
			if (!bin_is_empty(other)) {
				bin_union(accumulation, other);

				M_A = bbox_measure(accumulation.box);
				M_O = bcone_measure(accumulation.cone);
				E = accumulation.energy;
				N = accumulation.count;
			}

			data_out.M_A[i] = M_A;
			data_out.M_O[i] = M_O;
			data_out.E[i] = E.x + E.y + E.z;
			data_out.N[i] = N;
		}
		return M_A * M_O;
	}
	inline float LightTree::accumulate_from_right(bin_accumulation_data& data_out, const bin_data& bins)
	{
		constexpr int last = K - 1;

		// Initialize accumulative data with first bin
		bin accumulation = {};
		bin_init(accumulation);

		int N = 0;
		glm::vec3 E = glm::vec3(std::numeric_limits<float>::infinity());
		float M_A = std::numeric_limits<float>::infinity();
		float M_O = std::numeric_limits<float>::infinity();

		// Handle rest of the bins usually
		for (int i = last; i >= 0; i--) {
			const bin& other = bins.data[i];
			if (!bin_is_empty(other)) {
				bin_union(accumulation, other);

				M_A = bbox_measure(accumulation.box);
				M_O = bcone_measure(accumulation.cone);
				E = accumulation.energy;
				N = accumulation.count;
			}

			data_out.M_A[i] = M_A;
			data_out.M_O[i] = M_O;
			data_out.E[i] = E.x + E.y + E.z;
			data_out.N[i] = N;
		}
		return M_A * M_O;
	}
	inline void LightTree::calculate_splits(split_data& data_out, const bin_data& bins)
	{
		constexpr int last = K - 1;

		// allocate bin for accumulaton
		bin accumulation = {};

		// Accumulate from left to right
		bin_init(accumulation);
		for (int i = 0; i < last; i++) {
			// Fetch left bin in the split i
			const bin& other = bins.data[i];
			// Accumulate with previous bins
			bin_union(accumulation, other);
			// accumulated bin to the left bin in the split
			data_out.data[i].bin_l = accumulation;
		}

		// Accumulate from right to left
		bin_init(accumulation);
		for (int i = last; i > 0; i--) {
			// Fetch the right bin in the split i
			const bin& other = bins.data[i];
			// Accumulate with previous bins
			bin_union(accumulation, other);
			// Save accumulation to the right bin in split i
			data_out.data[i-1].bin_r = accumulation;
		}
	}
	inline int LightTree::find_best_split(const split_data& splits, const float K_r)
	{
		const float M_a = bbox_measure(splits.data[0].bin_r.box);
		const float M_o = bcone_measure(splits.data[0].bin_r.cone);

		// Find the best split
		float cost_best = std::numeric_limits<float>::infinity();
		int index_best = -1;
		for (int i = 0; i < K - 1; i++) {
			const bin& left = splits.data[i].bin_l;
			const bin& right = splits.data[i].bin_r;

			const float M_al = bbox_measure(left.box);
			const float M_ol = bcone_measure(left.cone);
			const float E_l = left.energy.x + left.energy.y + left.energy.z;
			const uint N_l = left.count;

			const float M_ar = bbox_measure(right.box);
			const float M_or = bcone_measure(right.cone);
			const float E_r = right.energy.x + right.energy.y + right.energy.z;
			const uint N_r = right.count;

			const float cost = K_r * (((E_l * M_al * M_ol) + (E_r * M_ar * M_or)) / (M_a * M_o));

			if (N_l == 0 || N_r == 0) {
				continue;
			}

			if (cost < cost_best) {
				cost_best = cost;
				index_best = i;
			}
		}

		CORE_ASSERT(index_best != -1, "Failed to find a valid split!");
		//printf("Best split: %d, cost: %f\n", index_best, cost_best);
		return index_best;
	}
	inline int LightTree::reorder_id(build_data& data, uint start, uint end, const uint32_t split_id, int k, float k0, float k1)
	{
		uint middle = partition(data, start, end, split_id + 1, k, k0, k1);
		return middle;
	}
	/// Hoare partition scheme
	inline int LightTree::partition(build_data& data, uint start, uint end, const uint split, int k, float k0, float k1)
	{
		CORE_ASSERT(start < end, "Range has to be non zero!");

		// calculate bin ids
		const int range = end - start;
		uint* bin_ids = new uint[end - start];
		for (int i = 0; i < range; i++) {
			const float c_ik = data.centers[data.ids[start + i]][k];
			uint bin_id = static_cast<uint>(k1 * (c_ik - k0));
			CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin id out of bounds!");
			bin_ids[i] = bin_id;
		}

		int left = 0;
		int right = range - 1;

		while (true) {
			while (bin_ids[left] < split) {
				if (left == right)
					return start + left + 1;
				left++;
			}
			while (bin_ids[right] >= split) {
				if (left == right) {
					return start + left;
				}
				right--;
			}
			std::swap(data.ids[start + left], data.ids[start + right]);
			right--;
			if (left == right)
				return start + left + 1;
			left++;
		}
	}
	inline LightTree::bbox LightTree::make_bbox()
	{
		bbox b = {};
		b.pmin = float3(std::numeric_limits<float>::infinity());
		b.pmax = float3(-std::numeric_limits<float>::infinity());
		return b;
	}
	inline LightTree::bbox LightTree::make_bbox(const glm::vec3 p)
	{
		bbox b = {};
		b.pmin = p;
		b.pmax = p;
		return b;
	}
	inline LightTree::bbox LightTree::make_bbox(const glm::vec3 pmin, const glm::vec3 pmax)
	{
		bbox box = {};
		box.pmin = pmin;
		box.pmax = pmax;
		return box;
	}
	inline LightTree::bcone LightTree::make_bcone(const glm::vec3 axis, float theta_o, float theta_e)
	{
		bcone b = {};
		b.axis = axis;
		b.theta_o = theta_o;
		b.theta_e = theta_e;
		return b;
	}
	inline void LightTree::swap(bcone& a, bcone& b)
	{
		bcone tmp = a;
		a = b;
		b = tmp;
	}
	inline LightTree::bbox LightTree::union_bbox(bbox a, bbox b)
	{
		bbox bound = {};
		bound.pmin = glm::min(a.pmin, b.pmin);
		bound.pmax = glm::max(a.pmax, b.pmax);
		return bound;
	}
	inline LightTree::bcone LightTree::union_bcone(bcone a, bcone b)
	{
		if (b.theta_o > a.theta_o) {
			swap(a, b);
		}
		const float theta_d = glm::acos(glm::dot(a.axis, b.axis));
		const float theta_e = glm::max(a.theta_e, b.theta_e);
		if (glm::min(theta_d + b.theta_o, glm::pi<float>()) <= a.theta_o) {
			return make_bcone(a.axis, a.theta_o, theta_e);
		}
		else {
			const float theta_o = (a.theta_o + theta_d + b.theta_o) / 2.0f;
			if (glm::pi<float>() <= theta_o) {
				return make_bcone(a.axis, glm::pi<float>(), theta_e);
			}

			const float theta_r = theta_o - a.theta_o;
			float3 axis = glm::rotate(a.axis, theta_r, glm::cross(a.axis, b.axis));
			return make_bcone(axis, theta_o, theta_e);
		}
	}
	/// returns the area of the bounding box
	inline float LightTree::bbox_measure(bbox b)
	{
		float3 d = b.pmax - b.pmin;
		return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
	}
	inline float LightTree::bcone_measure(bcone b)
	{
		constexpr float PI = glm::pi<float>();
		constexpr float M_2_PI = 2.0f * PI;
		constexpr float M_PI_2 = PI / 2.0f;

		// Short-hand for variables to avoid big equation
		const float t_o = b.theta_o;
		const float t_e = b.theta_e;
		const float t_w = glm::min(t_o + t_e, PI);

		return M_2_PI * (1.0f - cos(t_o)) + M_PI_2 * (2.0f * t_w * sin(t_o) - cos(t_o - 2.0f * t_w) - 2.0f * t_o * sin(t_o) + cos(t_o));
	}
	inline void LightTree::init_bins(bin_data& bin)
	{
		for (int i = 0; i < K; i++) {
			bin_init(bin.data[i]);
		}
	}
	inline void LightTree::bin_init(bin& data)
	{
		data.count = 0;
	}
	inline void LightTree::bin_union(bin& dst, const bin& other)
	{
		// if the other bin is empty, there's no need to do anything
		if (bin_is_empty(other)) {
			return;
		}

		// if the dst bin is empty just override with the other bin
		if (bin_is_empty(dst)) {
			dst = other;
		}
		else { // perform actual union
			dst.cb = union_bbox(dst.cb, other.cb);
			dst.box = union_bbox(dst.box, other.box);
			dst.cone = union_bcone(dst.cone, other.cone);
			dst.count += other.count;
			dst.energy += other.energy;
		}
	}
	inline void LightTree::bin_update(bin& data, const bbox& cb, const bbox& box, const bcone& cone, const glm::vec3& energy)
	{
		if (bin_is_empty(data)) {
			data.cb = cb;
			data.box = box;
			data.cone = cone;
			data.energy = energy;
		}
		else {
			data.cb = union_bbox(data.cb, cb);
			data.box = union_bbox(data.box, box);
			data.cone = union_bcone(data.cone, cone);
			data.energy += energy;
		}
		data.count++;
	}
	inline bool LightTree::bin_is_empty(const bin& data)
	{
		return data.count == 0;
	}
	inline LightTree::bound LightTree::calc_light_bounds(const SHARED::Light* lights, const uint index, const build_data& data)
	{
		const SHARED::Light& light = lights[index];

		// Load data from light source
		const glm::vec3 t = convert(light.tangent);
		const glm::vec3 b = convert(light.bitangent);

		const float area = glm::length(glm::cross(t, b)) * 0.5f;

		const glm::vec3 p0 = convert(light.position);
		const glm::vec3 p1 = p0 + t;
		const glm::vec3 p2 = p0 + b;

		const glm::vec3 pmin = glm::min(glm::min(p0, p1), p2);
		const glm::vec3 pmax = glm::max(glm::max(p0, p1), p2);
		const glm::vec3 center = (pmin + pmax) * 0.5f;

		const glm::vec3 axis = convert(light.direction);

		constexpr float theta_o = 0.0f;
		constexpr float theta_e = glm::pi<float>() / 2.0f;
		const float energy = (light.intensity.x + light.intensity.y + light.intensity.z) * area;

		// Save local bounds data
		data.pmin[index] = convert(pmin);
		data.pmax[index] = convert(pmax);
		data.centers[index] = convert(center);
		data.axis[index] = axis;
		data.theta_o[index] = theta_o;
		data.theta_e[index] = theta_e;
		data.energy[index] = energy;
		data.ids[index] = index;

		bound res = {};
		res.spatial = make_bbox(pmin, pmax);
		res.oriental = make_bcone(axis, theta_o, theta_e);
		return res;
	}
	/// Returns the index of the axis with the maximum value
	inline LightTree::uint LightTree::max_axis(float3 a)
	{
		return a.x > a.y ? (a.x > a.z ? 0 : 2) : (a.y > a.z ? 1 : 2);
	}
}