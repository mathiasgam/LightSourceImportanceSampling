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
	OUT_BUF(ulong, codes)
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
		ulong code = MortonCode(p);

		// Save result
		codes[id] = code;
	}

}

inline uint RadixByte(ulong x, uint shift) {
	return (x >> shift) & 0xff;
}

inline uint RadixBit(ulong x, uint shift) {
	return (x >> shift) & 0b1;
}

#define SORT_GROUP_SIZE 1

__attribute__((work_group_size_hint(SORT_GROUP_SIZE, 1, 1)))
__kernel void sort_codes(
	IN_VAL(uint, N),
	IN_VAL(uint, shift),
	IN_BUF(morton_key, codes),
	OUT_BUF(morton_key, codes_sorted)
) {
	const uint id = get_global_id(0);
	const uint id_local = get_local_id(0);

	const uint stride = N / SORT_GROUP_SIZE + (N % SORT_GROUP_SIZE > 0);
	const uint start = stride * id_local;
	const uint end = min(start + stride, N);

	if (id == 0) {
		uint count_table[256];
		for (uint i = 0; i < 256; i++) {
			count_table[i] = 0;
		}

		for (uint i = 0; i < N; i++) {
			morton_key code = codes[i];
			uint prefix = RadixByte(code.code, shift);
			count_table[prefix]++;
		}

		uint offset_table[256];
		offset_table[0] = 0;
		for (uint i = 1; i < 256; i++) {
			offset_table[i] = offset_table[i - 1] + count_table[i - 1];
		}

		for (uint i = 0; i < N; i++) {
			morton_key code = codes[i];
			uint prefix = RadixByte(code.code, shift);
			uint offset = offset_table[prefix]++;
			codes_sorted[offset] = code;
		}
	}
}

__kernel void generate_hierachy() {
}