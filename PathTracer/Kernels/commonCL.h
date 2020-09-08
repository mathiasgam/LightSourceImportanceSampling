#ifndef COMMON_CL
#define COMMON_CL

#include "shared_defines.h"

// This file is NOT meant to be included in CPP code. (only CL kernels)

#define IN_VAL(type, name) const type name
#define IN_BUF(type, name) __global const type* const restrict name
#define OUT_BUF(type, name) __global type* restrict name
#define CONST_BUF(type, name) __constant type* restrict name 

#define PI 3.14159265359f
#define TWO_PI (3.14159265359f * 2.0f)

#define STATE_INACTIVE 0
#define STATE_ACTIVE 1

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

inline Ray CreateRay(float3 origin, float3 dir, float tmin, float tmax) {
	Ray ray = {};
	ray.origin = (float4)(origin.xyz, tmin);
	ray.direction = (float4)(normalize(dir.xyz), tmax);
	return ray;
}

#define GetVertexUV(vertex) (float2)(vertex.position.w, vertex.normal.w)
#define GetVertexNormal(vertex) vertex.normal.xyz
#define GetVertexPosition(vertex) vertex.position.xyz

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

inline uint random_uint(uint* state, uint range){
	uint t = hash1(*state);
	*state = t;
	return t % range;
}

#define random_float2(state) (float2)(rand(state), rand(state))
#define random_float3(state) (float3)(rand(state), rand(state), rand(state))
#define random_float4(state) (float4)(rand(state), rand(state), rand(state), rand(state))

#define min3(x,y,z) min(x, min(y,z))
#define max3(x,y,z) max(x, max(y,z))

#define interpolate(f0,f1,f2,uv) mix(mix(f0,f1, uv.x), f2, uv.y)

inline float3 sample_hemisphere(uint* state, float3 normal) {
	float3 vec;
	float sqr_dist;
	do {
		// project random vector range [0,1] into [-1,1]
		vec = random_float3(state) * 2.0f - 1.0f;
		sqr_dist = dot(vec,vec);
	} while (sqr_dist > 1.0f && dot(normal, vec) > 0.0001f);
	return vec / sqrt(sqr_dist);
}

inline float2 sample_disk_uniform(uint* state){
	while (true)
	{
		float2 p = random_float2(state) * 2.0f - 1.0f;
		float sqr_dist = p.x * p.x + p.y * p.y;
		if (sqr_dist < 1.0f){
			return p;
		}
	}
}

float3 sample_hemisphere_cosine(uint* state, float3 normal)
{
    float phi = TWO_PI * rand(state);
    float sinThetaSqr = rand(state);
    float sinTheta = sqrt(sinThetaSqr);

    float3 axis = fabs(normal.x) > 0.001f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
    float3 t = normalize(cross(axis, normal));
    float3 s = cross(normal, t);

    return normalize(s*cos(phi)*sinTheta + t*sin(phi)*sinTheta + normal*sqrt(1.0f - sinThetaSqr));
}


#endif // COMMON_CL