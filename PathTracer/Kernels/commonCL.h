#ifndef COMMON_CL
#define COMMON_CL

#include "shared_defines.h"

// This file is NOT meant to be included in CPP code. (only CL kernels)

#define IN_VAL(type, name) const type name
#define IN_BUF(type, name) __global const type* const restrict name
#define OUT_BUF(type, name) __global type* restrict name

typedef struct mat4
{
	float4 x;
	float4 y;
	float4 z;
	float4 w;
} mat4;

float4 mul(const mat4 mat, const float4 f) {
	return (float4)(dot(mat.x, f), dot(mat.y, f), dot(mat.z, f), dot(mat.w, f));
}

Ray CreateRay(float3 origin, float3 dir, float tmin, float tmax) {
	Ray ray = {};
	ray.origin = (float4)(origin.xyz, tmin);
	ray.direction = (float4)(normalize(dir.xyz), tmax);
	return ray;
}

Intersection CreateIntersectionInfo(int num_hits, int closest_primitive, float2 uv, float tmax) {
	Intersection hit = {};
	hit.hit = num_hits;
	hit.primid = closest_primitive;
	hit.uvwt = (float4)(uv, 0.0f, tmax);
	return hit;
}

inline float2 GetVertexUV(Vertex v){
	return (float2)(v.position.w, v.normal.w);
}

inline float3 GetVertexNormal(Vertex v){
	return (float3)(v.normal.xyz);
}

inline float3 GetVertexPosition(Vertex v){
	return (float3)(v.position.xyz);
}

inline uint hash1(uint t) {
	t ^= t << 13;
	t ^= t >> 17;
	t ^= t << 5;
	return t;
}

inline uint hash2(uint t) {
	t += t << 10u;
	t ^= t >> 6u;
	t += t << 3u;
	t ^= t >> 11u;
	t += t << 15u;
	return t;
}

inline float uintToRangedFloat(uint x) {
	return (float)(x % 0xFFFF) / (float)(0xFFFF);
}

inline float rand(uint* state) {
	uint t = hash1(*state);
	*state = t;
	return uintToRangedFloat(t);
}

inline float2 random_float2(uint* state) {
	return (float2)(rand(state), rand(state));
}

inline float3 random_float3(uint* state) {
	return (float3)(rand(state), rand(state), rand(state));
}

inline float4 random_float4(uint* state) {
	return (float4)(rand(state), rand(state), rand(state), rand(state));
}

inline float min3(float x, float y, float z) {
	return min(x, min(y, z));
}

inline float max3(float x, float y, float z) {
	return max(x, max(y, z));
}
/*
inline float3 sample_hemisphere(uint* state) {
	float3 vec = random_float3(state);
	float sqr_dist = dot(vec, vec);

	while (sqr_dist > 1.0f) {
		vec = random_float3(state);
		sqr_dist = dot(vec, vec);
	}

	return normalize(vec);
}
*/

#endif // COMMON_CL