#include "pch.h"
#include "LightTree.h"

#include "gtc/constants.hpp"
#include "gtx/rotate_vector.hpp"

namespace LSIS {

	LightTree::LightTree(const SHARED::Light* lights, const size_t num_lights)
	{
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
			else { // Is Internal

				printf("index: %d, left: %d, right: %d\n", index, left, right);
				printf("cb: [%f,%f,%f][%f,%f,%f]\n", cb.pmin.x, cb.pmin.y, cb.pmin.y, cb.pmax.x, cb.pmax.y, cb.pmax.z);

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
					const float c_ik = data.pmin[id][k];
					const int bin_id = static_cast<uint32_t>(k1 * (c_ik - k0));
					CORE_ASSERT(bin_id >= 0 && bin_id < K, "Bin ID out of bounds!");

					if (bins.count[bin_id] == 0) {
						bins.box[bin_id] = make_bbox(data.pmin[id]);
						bins.cone[bin_id] = make_bcone(data.axis[id], data.theta_o[id], data.theta_e[id]);
						bins.count[bin_id] = 1;
						bins.energy[bin_id] = data.energy[id];
					}
					else {
						bins.box[bin_id] = union_bbox(bins.box[bin_id], make_bbox(data.pmin[id]));
						bins.cone[bin_id] = union_bcone(bins.cone[bin_id], make_bcone(data.axis[id], data.theta_o[id], data.theta_e[id]));
						bins.count[bin_id] += 1;
						bins.energy[bin_id] += data.energy[id];
					}
				}

				const float devisor = accumulate_from_left(bin_left, bins);
				const float devisor2 = accumulate_from_right(bin_right, bins);

				//CORE_ASSERT(devisor == devisor2, "Accumulation from left and right, should end with the same result");

				const int best_split = find_best_split(bin_left, bin_right, bins);

				bbox cb_l, cb_r;
				const int middle = reorder_id(data, left, right, &cb_l, &cb_r, best_split, k, k0, k1);

				//const int middle = left + (range / 2);
				const int index_left = next_index++;
				const int index_right = next_index++;

				const float3 pmin = data.pmin[left];
				const float3 pmax = data.pmax[left];
				const float3 axis = data.axis[left];
				const float theta_o = data.theta_o[left];
				const float theta_e = data.theta_e[left];

				m_nodes[index] = SHARED::make_light_tree_node(pmin, pmax, axis, theta_o, theta_e, index_left, index_right);

				printf("index: %d, left: %d, middle: %d, right: %d\n", index, left, middle, right);

				queue.push({ index_left, left, middle, cb_l});
				queue.push({ index_right, middle, right, cb_r });
			}
		}

		for (uint i = 0; i < next_index; i++) {
			const LightTreeNode& node = m_nodes[i];
			printf("Node: index: %d, left: %d, right: %d\n", i, node.left, node.right);
		}

		// Delete build data
		delete_build_data(data);
	}
	LightTree::~LightTree()
	{
		if (m_nodes)
			delete[] m_nodes;
	}
	inline LightTree::build_data LightTree::allocate_build_data(size_t size)
	{
		build_data data = {};
		data.pmin = new float3[size];
		data.pmax = new float3[size];
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
		delete[] data.axis;
		delete[] data.theta_o;
		delete[] data.theta_e;
		delete[] data.energy;
		delete[] data.ids;
	}
	inline LightTree::bound LightTree::initialize_build_data(build_data& data, const SHARED::Light* lights, const size_t num_lights)
	{
		// Allocate global bound struct
		bound cb = {};

		// Handle first light specially
		{
			const SHARED::Light light = lights[0];

			// Load data from light source
			const float3 position = float3(light.position.x, light.position.y, light.position.z);
			const float3 axis = float3(light.direction.x, light.direction.y, light.direction.z);
			constexpr float theta_o = glm::pi<float>();
			constexpr float theta_e = glm::pi<float>() / 2.0f;
			const float energy = light.intensity.x + light.intensity.y + light.intensity.z;

			// Save local bounds data
			data.pmin[0] = position;
			data.pmax[0] = position;
			data.axis[0] = axis;
			data.theta_o[0] = theta_o;
			data.theta_e[0] = theta_e;
			data.energy[0] = energy;
			data.ids[0] = 0;

			// Update global bounds
			cb.spatial = make_bbox(position);
			cb.oriental = make_bcone(axis, theta_o, theta_e);
		}

		// Handle rest of the lights
		for (int i = 1; i < num_lights; i++) {
			const SHARED::Light light = lights[i];

			// Load data from light source
			const float3 position = float3(light.position.x, light.position.y, light.position.z);
			const float3 axis = float3(light.direction.x, light.direction.y, light.direction.z);
			constexpr float theta_o = glm::pi<float>();
			constexpr float theta_e = glm::pi<float>() / 2.0f;
			const float energy = light.intensity.x + light.intensity.y + light.intensity.z;

			// Save local bounds data
			data.pmin[i] = position;
			data.pmax[i] = position;
			data.axis[i] = axis;
			data.theta_o[i] = theta_o;
			data.theta_e[i] = theta_e;
			data.energy[i] = energy;
			data.ids[i] = i;

			// Update global bounds
			cb.spatial = union_bbox(cb.spatial, make_bbox(position));
			cb.oriental = union_bcone(cb.oriental, make_bcone(axis, theta_o, theta_e));
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
			printf("Bin: %d, cost %f, N_l: %d, N_r: %d\n", i, cost, N_l, N_r);

			if (N_l == 0 || N_r == 0) {
				continue;
			}

			if (cost < cost_best) {
				cost_best = cost;
				index_best = i;
			}
		}
		printf("Best split: %d, cost: %f\n", index_best, cost_best);
		return index_best;
	}
	inline int LightTree::reorder_id(build_data& data, uint start, uint end, bbox* cb_l, bbox* cb_r, const uint32_t split_bin_id, int k, float k0, float k1)
	{
		int left = start;
		int right = end - 1;

		float3 pmin_l = float3(std::numeric_limits<float>::infinity());
		float3 pmin_r = float3(std::numeric_limits<float>::infinity());
		float3 pmax_l = float3(-std::numeric_limits<float>::infinity());
		float3 pmax_r = float3(-std::numeric_limits<float>::infinity());

		while (left < right) {
			uint id_l = data.ids[left];
			const float3 c_il = data.pmin[id_l];
			const int bin_id_l = static_cast<uint32_t>(k1 * (c_il[k] - k0));

			printf("L: %d, R: %d\n", left, right);

			if (bin_id_l > split_bin_id) {
				while (left < right) {
					uint id_r = data.ids[right];
					const float3 c_ir = data.pmin[id_r];
					const int bin_id_r = static_cast<uint32_t>(k1 * (c_ir[k] - k0));

					printf("L: %d, R: %d\n", left, right);

					if (bin_id_r <= split_bin_id) {
						data.ids[left] = id_r;
						data.ids[right] = id_l;

						printf("swap %d, %d\n", left, right);

						pmin_l = glm::min(pmin_l, c_ir);
						pmax_l = glm::max(pmax_l, c_ir);
						pmin_r = glm::min(pmin_r, c_il);
						pmax_r = glm::max(pmax_r, c_il);
						right--;
						break;
					}
					else {
						pmin_r = glm::min(pmin_r, c_ir);
						pmax_r = glm::max(pmax_r, c_ir);
						right--;
					}
				}
			}
			else {
				pmin_l = glm::min(pmin_l, c_il);
				pmax_l = glm::max(pmax_l, c_il);
			}

			left++;
		}

		if (left == right) {
			uint id = data.ids[right];
			const float3 c_i = data.pmin[id];
			const int bin_id = static_cast<uint32_t>(k1 * (c_i[k] - k0));

			if (bin_id > split_bin_id) {
				pmin_r = glm::min(pmin_r, c_i);
				pmax_r = glm::max(pmax_r, c_i);
				right--;
			} else {
				pmin_l = glm::min(pmin_l, c_i);
				pmax_l = glm::max(pmax_l, c_i);
				left++;
			}
		}

		// Save center bounds for use outside of funtion
		cb_l->pmin = pmin_l;
		cb_l->pmax = pmax_l;
		cb_r->pmin = pmin_r;
		cb_r->pmax = pmax_r;

		return left;
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
	/// Returns the index of the axis with the maximum value
	inline LightTree::uint LightTree::max_axis(float3 a)
	{
		return a.x > a.y ? (a.x > a.z ? 0 : 2) : (a.y > a.z ? 1 : 2);
	}
}