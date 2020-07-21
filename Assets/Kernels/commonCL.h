#ifndef COMMON_CL
#define COMMON_CL

#include "shared_defines.h"

// This file is NOT meant to be included in CPP code. (only CL kernels)

#define IN_VAL(type, name) const type name
#define IN_BUF(type, name) __global const type* const restrict name
#define OUT_BUF(type, name) __global type* const restrict name

typedef struct mat4
{
	float4 x;
	float4 y;
	float4 z;
	float4 w;
} mat4;

float4 mul(const mat4 mat, const float4 f) {
	//float4 row0 = (float4)(mat.x.x, mat.y.x, mat.z.x, mat.w.x);
	//float4 row1 = (float4)(mat.x.y, mat.y.y, mat.z.y, mat.w.y);
	//float4 row2 = (float4)(mat.x.z, mat.y.z, mat.z.z, mat.w.z);
	//float4 row3 = (float4)(mat.x.w, mat.y.w, mat.z.w, mat.w.w);

	return (float4)(dot(mat.x, f), dot(mat.y, f), dot(mat.z, f), dot(mat.w, f));
	//return mat.x * f.x + mat.y * f.y + mat.z * f.z + mat.w * f.w;
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

#endif // COMMON_CL