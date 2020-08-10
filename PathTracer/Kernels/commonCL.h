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

inline float min3(float x, float y, float z) {
	return min(x, min(y, z));
}

inline float max3(float x, float y, float z) {
	return max(x, max(y, z));
}

inline float3 min3(float3 x, float3 y, float3 z) {
	return min(x, min(y, z));
}

inline float3 max3(float3 x, float3 y, float3 z) {
	return max(x, max(y, z));
}

inline float4 min3(float4 x, float4 y, float4 z) {
	return min(x, min(y, z));
}

inline float4 max3(float4 x, float4 y, float4 z) {
	return max(x, max(y, z));
}

#endif // COMMON_CL