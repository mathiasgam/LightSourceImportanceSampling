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

		bin_data bins = {};
		bin_accumulation_data bin_left = {};
		bin_accumulation_data bin_right = {};

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
				const float theta_o = data.theta_o[id];
				const float theta_e = data.theta_e[id];

				m_nodes[index] = SHARED::make_light_tree_node(pmin, pmax, axis, theta_o, theta_e, -1, left);
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

				bbox box = union_bbox(box_left, box_right);
				bcone cone = union_bcone(cone_left, cone_right);

				m_nodes[index] = SHARED::make_light_tree_node(box.pmin, box.pmax, cone.axis, cone.theta_o, cone.theta_e, index_left, index_right);

				queue.push({ index_left, left, middle, box_left });
				queue.push({ index_right, middle, right, box_right });
			}
			else { // Is Internal

				
				const glm::vec3 diagonal = cb.pmax - cb.pmin;
				//printf("cb: [%f,%f,%f][%f,%f,%f] d: [%f,%f,%f]\n", cb.pmin.x, cb.pmin.y, cb.pmin.z, cb.pmax.x, cb.pmax.y, cb.pmax.z, diagonal.x, diagonal.y, diagonal.z);

				for (int i = 0; i < K; i++) {
					bins.count[i] = 0;
					bins.energy[i] = 0.0f;
				}

				const uint k = max_axis(cb.pmax - cb.pmin);
				const float k0 = cb.pmin[k];
				const float k1 = (static_cast<float>(K)* (1.0f - 1e-6f)) / (cb.pmax[k] - cb.pmin[k]);

				// Calculate bins
				for (int i = left; i < right; i++) {
					const uint id = data.ids[i];
					const float c_ik = data.centers[id][k];
					if (cb.pmin[k] > c_ik || cb.pmax[k] < c_ik) {
						printf("ERROR: k: %d, pmin: %f, pmax: %f, c_ik: %f\n", k, cb.pmin[k], cb.pmax[k], c_ik);
					}
					const int bin_id = static_cast<uint32_t>(k1 * (c_ik - k0));
					CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");

					if (bins.count[bin_id] == 0) {
						bins.box[bin_id] = make_bbox(data.centers[id]);
						bins.cone[bin_id] = make_bcone(data.axis[id], data.theta_o[id], data.theta_e[id]);
						bins.count[bin_id] = 1;
						bins.energy[bin_id] = data.energy[id];
					}
					else {
						bins.box[bin_id] = union_bbox(bins.box[bin_id], make_bbox(data.centers[id]));
						bins.cone[bin_id] = union_bcone(bins.cone[bin_id], make_bcone(data.axis[id], data.theta_o[id], data.theta_e[id]));
						bins.count[bin_id] += 1;
						bins.energy[bin_id] += data.energy[id];
					}
				}

				accumulate_from_left(bin_left, bins);
				accumulate_from_right(bin_right, bins);

				const int best_split = find_best_split(bin_left, bin_right, bins);

				bbox cb_l, cb_r;
				const int middle = reorder_id(data, left, right, &cb_l, &cb_r, best_split+1, k, k0, k1);

				//const int middle = left + (range / 2);
				const int index_left = next_index++;
				const int index_right = next_index++;

				const float3 pmin = data.pmin[left];
				const float3 pmax = data.pmax[left];
				const float3 axis = data.axis[left];
				const float theta_o = data.theta_o[left];
				const float theta_e = data.theta_e[left];

				m_nodes[index] = SHARED::make_light_tree_node(pmin, pmax, axis, theta_o, theta_e, index_left, index_right);

				//printf("index: %d, left: %d, middle: %d, right: %d\n", index, left, middle, right);

				CORE_ASSERT(left != middle && middle != right, "only non zero ranges are allowed!");

				queue.push({ index_left, left, middle, cb_l});
				queue.push({ index_right, middle, right, cb_r });
			}
		}

		/*
		for (uint i = 0; i < next_index; i++) {
			const LightTreeNode& node = m_nodes[i];
			printf("Node: index: %d, left: %d, right: %d\n", i, node.left, node.right);
		}
		*/

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
		// Initialize accumulative data with first bin
		bbox box = {};
		bcone cone = {};
		uint N = 0;

		float E = std::numeric_limits<float>::infinity();
		float M_A = std::numeric_limits<float>::infinity();
		float M_O = std::numeric_limits<float>::infinity();

		// Handle rest of the bins usually
		for (int i = 0; i < K; i++) {
			const uint N_i = bins.count[i];
			if (N_i > 0) { // if current bin is non empty, update accumulation data
				if (N > 0) { // unify accumulated bin and current bin
					box = union_bbox(box, bins.box[i]);
					cone = union_bcone(cone, bins.cone[i]);
					E += bins.energy[i];
				}
				else { // if accumulated bin is empty initialize with current bin
					box = bins.box[i];
					cone = bins.cone[i];
					E = bins.energy[i];
				}
				M_A = bbox_measure(box);
				M_O = bcone_measure(cone);
				N += N_i;
			}
			data_out.M_A[i] = M_A;
			data_out.M_O[i] = M_O;
			data_out.E[i] = E;
			data_out.N[i] = N;
		}
		return M_A * M_O;
	}
	inline float LightTree::accumulate_from_right(bin_accumulation_data& data_out, const bin_data& bins)
	{
		constexpr int last = K - 1;

		// Initialize accumulative data with first bin
		bbox box = {};
		bcone cone = {};
		uint N = 0;

		float E = std::numeric_limits<float>::infinity();
		float M_A = std::numeric_limits<float>::infinity();
		float M_O = std::numeric_limits<float>::infinity();

		// Handle rest of the bins usually
		for (int i = last; i >= 0; i--) {
			const uint N_i = bins.count[i];
			if (N_i > 0) {
				if (N > 0) {
					box = union_bbox(box, bins.box[i]);
					cone = union_bcone(cone, bins.cone[i]);
					E += bins.energy[i];
				}
				else {
					box = bins.box[i];
					cone = bins.cone[i];
					E = bins.energy[i];
				}
				M_A = bbox_measure(box);
				M_O = bcone_measure(cone);
				N += N_i;
			}
			data_out.M_A[i] = M_A;
			data_out.M_O[i] = M_O;
			data_out.E[i] = E;
			data_out.N[i] = N;
		}
		return M_A * M_O;
	}
	inline int LightTree::find_best_split(const bin_accumulation_data& bin_left, const bin_accumulation_data& bin_right, const bin_data& bins)
	{
		CORE_ASSERT(bin_left.M_A[K - 1] == bin_right.M_A[0], "Last accumulated measure should be equal to the total measure!");

		const float K_r = 1.0f;
		const float M_a = bin_right.M_A[0];
		const float M_o = bin_right.M_O[0];

		// Find the best split
		float cost_best = std::numeric_limits<float>::infinity();
		int index_best = 0;
		for (int i = 0; i < K-1; i++) {
			const float M_al = bin_left.M_A[i];
			const float M_ol = bin_left.M_O[i];
			const float E_l = bin_left.E[i];
			const uint N_l = bin_left.N[i];

			const float M_ar = bin_right.M_A[i+1];
			const float M_or = bin_right.M_O[i+1];
			const float E_r = bin_right.E[i+1];
			const uint N_r = bin_right.N[i+1];

			const float cost = K_r * (((E_l * M_al * M_ol) + (E_r * M_ar * M_or)) / (M_a * M_o));
			//printf("Bin: %d, cost %f, N_l: %d, N_r: %d\n", i, cost, N_l, N_r);
			//printf("M_al: %f, M_ol: %f, E_l: %f, M_ar: %f, M_or: %f, E_r: %f\n", M_al, M_ol, E_l, M_ar, M_or, E_r);

			if (N_l == 0 || N_r == 0) {
				continue;
			}

			if (cost < cost_best) {
				cost_best = cost;
				index_best = i;
			}
		}
		//printf("Best split: %d, cost: %f\n", index_best, cost_best);
		return index_best;
	}
	inline int LightTree::reorder_id(build_data& data, uint start, uint end, bbox* cb_l, bbox* cb_r, const uint32_t split_bin_id, int k, float k0, float k1)
	{
		uint middle = partition(data, start, end, split_bin_id, k, k0, k1);

		glm::vec3 pmin_l = glm::vec3(std::numeric_limits<float>::infinity());
		glm::vec3 pmax_l = glm::vec3(-std::numeric_limits<float>::infinity());
		for (int i = start; i < middle; i++) {
			const glm::vec3 center = data.centers[data.ids[i]];
			CORE_ASSERT(static_cast<uint>(k1 * (center[k] - k0)) < split_bin_id, "Partition failed. id should have been to the right!");

			pmin_l = glm::min(pmin_l, center);
			pmax_l = glm::max(pmax_l, center);
		}
		cb_l->pmin = pmin_l;
		cb_l->pmax = pmax_l;

		glm::vec3 pmin_r = glm::vec3(std::numeric_limits<float>::infinity());
		glm::vec3 pmax_r = glm::vec3(-std::numeric_limits<float>::infinity());
		for (int i = middle; i < end; i++) {
			const glm::vec3 center = data.centers[data.ids[i]];
			CORE_ASSERT(static_cast<uint>(k1 * (center[k] - k0)) >= split_bin_id, "Partition failed. id should have been to the left!");
			pmin_r = glm::min(pmin_r, center);
			pmax_r = glm::max(pmax_r, center);
		}
		cb_r->pmin = pmin_r;
		cb_r->pmax = pmax_r;
		
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
	inline LightTree::bound LightTree::calc_light_bounds(const SHARED::Light* lights, const uint index, const build_data& data)
	{
		const SHARED::Light& light = lights[index];

		// Load data from light source
		const glm::vec3 t = convert(light.tangent);
		const glm::vec3 b = convert(light.bitangent);

		const glm::vec3 p0 = convert(light.position);
		const glm::vec3 p1 = p0 + t;
		const glm::vec3 p2 = p0 + b;

		const glm::vec3 pmin = glm::min(glm::min(p0, p1), p2);
		const glm::vec3 pmax = glm::max(glm::max(p0, p1), p2);
		const glm::vec3 center = (pmin + pmax) * 0.5f;

		const glm::vec3 axis = convert(light.direction);

		constexpr float theta_o = 0.0f;
		constexpr float theta_e = glm::pi<float>() / 2.0f;
		const float energy = light.intensity.x + light.intensity.y + light.intensity.z;

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