#ifndef SHARED_DEFINES
#define SHARED_DEFINES

#if defined(APP_LSIS) // in CPP
// define cl types to match hlsl
#include "CL/cl.h"
typedef cl_float4 float4;
typedef cl_float3 float3;
typedef cl_float2 float2;
typedef cl_uint4 uint4;
typedef cl_uint3 uint3;
typedef cl_uint2 uint2;
typedef cl_uint uint;
typedef cl_int4 int4;
typedef cl_int3 int3;
typedef cl_int2 int2;

namespace SHARED {
#else // in HLSL

#endif

    typedef struct Ray
    {
        float4 origin; // xyz is origin, w is t_min
        float4 direction; // xyz is direction, w is t_max
    } Ray;

    typedef struct Intersection
    {
        int hit;
        int primid;
        // Padding elements
        int pixel_index;
        int padding0;

        // uv - hit barycentrics, w - ray distance
        float4 uvwt;
    } Intersection;

    // Defines types for the buffers
    typedef struct Vertex {
        float4 position;
        float4 normal;
    } Vertex;

    typedef struct Face {
        uint4 index;
    } Face;

    typedef struct Node {
        int parent;
        int left;
        int right;
        int next;
    } Node;

    typedef struct AABB {
        float4 min;
        float4 max;
    } AABB;

    typedef struct LightSample {
        float4 throughput;
        float4 light_color;
    } LightSample;

    typedef struct Sample {
        float4 position;
        float4 normal;
        float4 incoming;
        float4 throughput;
    } Sample;

    typedef struct Pixel {
        float4 color;
    } Pixel;

#if defined(APP_LSIS)
}
#endif

#endif // !SHARED_DEFINES