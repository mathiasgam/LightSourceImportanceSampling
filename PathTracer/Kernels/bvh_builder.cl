#include "commonCL.h"

typedef struct morton_key {
	ulong code;
	uint index;
} morton_key;

inline ulong SplitBy3(ulong x)
{
	x &= 0x1fffff; // we only look at the first 21 bits
	x = (x | x << 32) & 0x1f00000000ffff;
	x = (x | x << 16) & 0x1f0000ff0000ff;
	x = (x | x << 8) & 0x100f00f00f00f00f;
	x = (x | x << 4) & 0x10c30c30c30c30c3;
	x = (x | x << 2) & 0x1249249249249249;

	return x;
}

inline ulong MortonCode(float3 p) {

	// points must be in the range [0,1]
	//assert(p[0] <= 1 && p[0] >= 0); // x-component out of range [0,1]
	//assert(p[1] <= 1 && p[1] >= 0); // y-component out of range [0,1]
	//assert(p[2] <= 1 && p[2] >= 0); // z-component out of range [0,1]

	// project the normalized values onto the range of 21 bits
	float3 tmp = p * (float)(0x1fffff);

	ulong x = SplitBy3((ulong)(tmp.x));
	ulong y = SplitBy3((ulong)(tmp.y));
	ulong z = SplitBy3((ulong)(tmp.z));

	// interleave the bits from the 3 dimensions
	return x | y << 1 | z << 2;

}

__kernel void prepare_geometry_data(
	IN_VAL(uint, num_vertices),
	IN_VAL(uint, num_faces),
	IN_BUF(Vertex, vertices),
	IN_BUF(Face, faces),
	OUT_BUF(float3, centers),
	OUT_BUF(float3, p_mins),
	OUT_BUF(float3, p_maxs)
) {
	const uint id = get_global_id(0);
	if (id < num_faces) {

		// Fetch face geometry 
		Face face = faces[id];
		float3 v0 = vertices[face.index.x].position.xyz;
		float3 v1 = vertices[face.index.y].position.xyz;
		float3 v2 = vertices[face.index.z].position.xyz;

		float3 center = (v0 + v1 + v2) / 3.0f;
		float3 p_min = min(v0, min(v1, v2));
		float3 p_max = max(v0, max(v1, v2));

		centers[id] = center;
		p_mins[id] = p_min;
		p_maxs[id] = p_max;
	}
}

#define WORKGROUP_SIZE 32

__attribute__((work_group_size_hint(WORKGROUP_SIZE, 1, 1)))
__kernel void calc_scene_bounds(
	IN_VAL(uint, N),
	IN_BUF(float3, p_mins),
	IN_BUF(float3, p_maxs),
	OUT_BUF(float3, bounds) // [p_min, p_max]
) {
	__local float3 min_array[WORKGROUP_SIZE];
	__local float3 max_array[WORKGROUP_SIZE];

	const uint id = get_global_id(0);
	const uint id_local = get_local_id(0);

	const uint stride = N / WORKGROUP_SIZE + (N % WORKGROUP_SIZE > 0);
	const uint start = stride * id_local;
	const uint end = min(start + stride, N);

	float3 p_min = (float3)(INFINITY);
	float3 p_max = (float3)(-INFINITY);

	for (uint i = start; i < end; i++) {
		p_min = min(p_min, p_mins[i]);
		p_max = max(p_max, p_maxs[i]);
	}

	min_array[id_local] = p_min;
	max_array[id_local] = p_max;

	barrier(CLK_LOCAL_MEM_FENCE);

	for (uint offset = WORKGROUP_SIZE / 2; offset > 0; offset >>= 1) {
		if (id_local < offset) {
			min_array[id_local] = min(min_array[id_local], min_array[id_local + offset]);
			max_array[id_local] = max(max_array[id_local], max_array[id_local + offset]);
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (id == 0) {
		bounds[0] = min_array[0];
		bounds[1] = max_array[0];
		//printf("Scene Bounds: [%f,%f,%f][%f,%f,%f]\n", min_array[0].x,min_array[0].y,min_array[0].z,max_array[0].x,max_array[0].y,max_array[0].z);
	}

}

__kernel void generate_morton_codes(
	IN_VAL(uint, N),
	IN_BUF(float3, centers),
	IN_BUF(float3, bounds),
	OUT_BUF(morton_key, codes)
) {
	const int id = get_global_id(0);

	// Check that the id is in range, to not overflow any buffers.
	if (id < N) {

		// Load global bounds for the scene into stack
		const float3 p_min = bounds[0];
		const float3 p_max = bounds[1];
		const float3 center = centers[id];

		// Calculate the scale and transform, to map the positions into the range [0.0,1.0]
		const float3 diagonal = p_max - p_min;
		const float scale = max(diagonal.x, max(diagonal.y, diagonal.z));
		const float3 transform = -p_min * scale;

		// calculate the morton code
		const float3 p = center * scale + transform;
		morton_key key = {};
		key.code = MortonCode(p);
		key.index = id;

		// Save result
		codes[id] = key;
	}

}

#define SORT_GROUP_SIZE 64
#define BYTE_MASK 0xff
#define BYTE_RANGE 256

inline uint RadixByte(ulong x, uint shift) {
	return (x >> shift) & BYTE_MASK;
}

__attribute__((work_group_size_hint(SORT_GROUP_SIZE, 1, 1)))
__kernel void sort_codes(
	IN_VAL(uint, N),
	IN_VAL(uint, shift),
	IN_BUF(morton_key, codes),
	OUT_BUF(morton_key, codes_sorted)
) {
	const uint id = get_global_id(0);
	const uint id_local = get_local_id(0);

	const uint stride = N / SORT_GROUP_SIZE + (N % SORT_GROUP_SIZE > 0) + 1;
	const uint start = stride * id_local;
	const uint end = min(start + stride, N);

	//printf("id: %d, range[%d,%d], N: %d\n", id, start, end, N);

	volatile __local uint count_global[BYTE_RANGE];
	__local uint offset_global[BYTE_RANGE];

	if (id == 0) {
		for (uint i = 0; i < BYTE_RANGE; i++) {
			count_global[i] = 0;
		}
	}

	uint count_table[BYTE_RANGE];
	for (uint i = 0; i < BYTE_RANGE; i++) {
		count_table[i] = 0;
	}

	for (uint i = start; i < end; i++) {
		morton_key key = codes[i];
		uint prefix = RadixByte(key.code, shift);
		count_table[prefix]++;
	}

	// calculate local offsets
	uint offset_table[BYTE_RANGE];
	for (uint i = 0; i < BYTE_RANGE; i++) {
		offset_table[i] = atomic_add(&count_global[i], count_table[i]);
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	// calculate global offsets of each key
	if (id == 0) {
		offset_global[0] = 0;
		for (uint i = 1; i < BYTE_RANGE; i++) {
			offset_global[i] = offset_global[i - 1] + count_global[i - 1];
			//printf("index: %d, count: %d, offset: %d\n", i, count_global[i], offset_global[i]);
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	// Add global offset to local offset
	for (uint i = 0; i < BYTE_RANGE; i++) {
		offset_table[i] += offset_global[i];
		//printf("index: %d, offset: %d\n", i, offset_table[i]);
	}

	for (uint i = start; i < end; i++) {
		morton_key key = codes[i];
		uint prefix = RadixByte(key.code, shift);
		uint offset = offset_table[prefix]++;
		codes_sorted[offset] = key;
	}

}

inline int longest_common_prefix(__global morton_key const* restrict morton_keys, int num_primitives, int i0, int i1) {
	// select left and right 
	int left = min(i0, i1);
	int right = max(i0, i1);

	// check the left and right is within the range of the primitives
	if (left < 0 || right >= num_primitives) {
		return -1;
	}

	// Load codes from buffer
	ulong left_code = morton_keys[left].code;
	ulong right_code = morton_keys[right].code;

	// in case the codes are the same, find common prefix for the indices instead.
	return left_code != right_code ? clz(left_code ^ right_code) : (64 + clz(left ^ right));
}

inline int2 find_span(__global morton_key const* restrict keys, int num_primitives, int index) {
	// direction for the range
	int d = sign((float)(longest_common_prefix(keys, index, index + 1) - longest_common_prefix(keys, index, index - 1)));

	int delta_min = longest_common_prefix(keys, index, index - d);

	// Max rough estimate for far end
	int lmax = 2;
	while (longest_common_prefix(keys, index, index + lmax * d) > delta_min)
		lmax *= 2;

	// Search back for the exact bound in the span
	int l = 0;
	int t = lmax;
	do {
		t /= 2;
		if (longest_common_prefix(keys, index, index + (l + t) * d) > delta_min) {
			l = l + t;
		}
	} while (t > 1);

	return (float2)(min(index, index + l * d), max(index, index + l * d));
}

inline int find_split(__global morton_key const* restrict keys, int num_primitives, int2 span) {
	int left = span.x;
	int right = span.y;

	int num_common = longest_common_prefix(keys, left, right);

	do {
		// propose new split in the middle of the range [left,right]
		int split = (right + left) / 2;

		if (longest_common_prefix(keys, left, split) > num_common) {
			left = split;
		}
		else {
			right = split;
		}

	} while (right > left + 1);

	return left;
}

#define LEAFIDX(i) ((num_prims-1) + i)
#define NODEIDX(i) (i)

__kernel void generate_hierachy(
	IN_VAL(uint, num_primitives),
	IN_VAL(uint, num_nodes),
	IN_BUF(morton_key, codes),
	IN_BUF(AABB, bboxes),
	OUT_BUF(Node, nodes),
	OUT_BUF(AABB, nodes_bboxes)
){
	const uint id = get_global_id(0);

	// Create leaf nodes
	if (id < num_primitives) {
		// Have all the leaves in the back half of the buffer
		const uint leaf_index = (num_primitives - 1) + id;
		Node node = {};
		node.left = -1;
		node.right = codes[id].index;
		nodes[leaf_index] = node;

		nodes_bboxes[leaf_index] = bboxes[id];
	}

	// Create internal nodes
	if (id < num_primitives - 1) {
		// find the range the current node covers
		int2 range = find_span(codes, num_primitives, id);

		// find the split position in that range
		int split = find_split(codes, num_primitives, range);

		// Create child nodes if necessary
		int child_left = (split == range.x) ? (num_primitives - 1) + split : split;
		int child_right = (split + 1 == range.y) ? num_primitives - split : split + 1;

		nodes[id].left = child_left;
		nodes[id].right = child_right;
		nodes[child_left].parent = id;
		nodes[child_right].parent = id;
	}

}

__kernel void refit_bounds(
	IN_VAL(uint, num_primitives),
	IN_BUF(Node, nodes),
	OUT_BUF(AABB, bboxes),
	OUT_BUF(uint, flags)
) {


}