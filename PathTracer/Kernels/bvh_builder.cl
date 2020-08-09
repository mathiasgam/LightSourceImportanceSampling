#include "commonCL.h"

inline ulong MortonCode(float3 p){
	return 0;
}

__kernel void prepare_geometry_data(
	IN_VAL(uint, num_vertices),
	IN_VAL(uint, num_faces),
	IN_BUF(Vertex, vertices),
	IN_BUF(Face, faces),
	OUT_BUF(float3, centers),
	OUT_BUF(float3, p_mins),
	OUT_BUF(float3, p_maxs)
){
	const uint id = get_global_id(0);
	if (id < num_faces){

		// Fetch face geometry 
		Face face = faces[id];
		float3 v0 = vertices[face.index.x].position.xyz;
        float3 v1 = vertices[face.index.y].position.xyz;
        float3 v2 = vertices[face.index.z].position.xyz;

		float3 center =  (v0 + v1 + v2) / 3.0f;
		float3 p_min = min(v0, min(v1,v2));
		float3 p_max = max(v0, max(v1,v2));
	
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
){
	__local float3 min_array[WORKGROUP_SIZE];
	__local float3 max_array[WORKGROUP_SIZE];

	const uint id = get_global_id(0);
	const uint id_local = get_local_id(0);

	const uint stride = N / WORKGROUP_SIZE + (N % WORKGROUP_SIZE > 0);
	const uint start = stride * id_local;
	const uint end = min(start + stride, N);

	float3 p_min = (float3)(INFINITY);
	float3 p_max = (float3)(-INFINITY);

	for (uint i = start; i < end; i++){
		p_min = min(p_min, p_mins[i]);
		p_max = max(p_max, p_maxs[i]);
	}

	min_array[id_local] = p_min;
	max_array[id_local] = p_max;

	barrier(CLK_LOCAL_MEM_FENCE);

	for (uint offset = WORKGROUP_SIZE / 2; offset > 0; offset >>= 1){
		if (id_local < offset){
			min_array[id_local] = min(min_array[id_local], min_array[id_local + offset]);
			max_array[id_local] = max(max_array[id_local], max_array[id_local + offset]);
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (id == 0){
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
){
	const int id = get_global_id(0);

	// Check that the id is in range, to not overflow any buffers.
	if (id < N){

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

__attribute__((work_group_size_hint(WORKGROUP_SIZE, 1, 1)))
__kernel void sort(
	IN_VAL(uint, N),
	OUT_BUF(uint, index),
	OUT_BUF(ulong, codes)
){
	const uint id = get_global_id(0);

	if (id == 0){
		printf("Sorting");
	}

}

__kernel void generate_hierachy(){
}