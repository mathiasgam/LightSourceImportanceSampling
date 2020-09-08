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
        int prim_index;
        // Padding elements
        int material_index;
        int padding0;
    } Intersection;

    // Defines types for the buffers
    typedef struct Vertex {
        float4 position;
        float4 normal;
    } Vertex;

    typedef struct Face {
        uint4 index;
    } Face;

    typedef struct Material {
        float4 diffuse;
        float4 specular;
    } Material;

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

    typedef struct GeometricInfo {
        float4 position;
        float4 normal;
        float4 incoming;
        float4 uvwt;
    } GeometricInfo;

    typedef struct Pixel {
        float4 color;
    } Pixel;

    typedef struct Light {
        float4 position;
        float4 direction;
        float4 intensity;
    } Light;

#if defined(APP_LSIS)

    inline Light make_light(glm::vec3 position, glm::vec3 direction, glm::vec3 intensity) {
        Light light = {};
        light.position = { position.x, position.y, position.z, 0.0f };
        light.direction = { direction.x, direction.y, direction.z, 0.0f };
        light.intensity = { intensity.x, intensity.y, intensity.z, 0.0f };
        return light;
    }

    inline Material make_material(glm::vec3 diffuse, glm::vec3 specular){
        Material material = {};
        material.diffuse = { diffuse.x, diffuse.y, diffuse.z, 1.0f };
        material.specular = { specular.x, specular.y, specular.z, 1.0f };
        return material;
    }

}
#endif

#endif // !SHARED_DEFINES