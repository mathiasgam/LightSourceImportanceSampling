#ifndef SHARED_DEFINES
#define SHARED_DEFINES

#if defined(APP_LSIS) // in CPP
// define cl types to match hlsl
#include "CL/cl.h"


namespace SHARED {
#else // in HLSL
typedef float4  cl_float4;
typedef float3  cl_float3;
typedef float2  cl_float2;
typedef uint4   cl_uint4;
typedef uint3   cl_uint3;
typedef uint2   cl_uint2;
typedef uint    cl_uint;
typedef int4    cl_int4;
typedef int3    cl_int3;
typedef int2    cl_int2;
#endif

    typedef struct Ray
    {
        cl_float4 origin; // xyz is origin, w is t_min
        cl_float4 direction; // xyz is direction, w is t_max
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
        cl_float4 position;
        cl_float4 normal;
    } Vertex;

    typedef struct Face {
        cl_uint4 index;
    } Face;

    typedef struct Material {
        cl_float4 diffuse;
        cl_float4 specular;
        cl_float4 emission;
    } Material;

    typedef struct Node {
        int parent;
        int left;
        int right;
        int next;
    } Node;

    typedef struct AABB {
        cl_float4 min;
        cl_float4 max;
    } AABB;

    typedef struct GeometricInfo {
        cl_float4 position;
        cl_float4 normal;
        cl_float4 incoming;
        cl_float4 uvwt;
    } GeometricInfo;

    typedef struct Pixel {
        cl_float4 color;
    } Pixel;

    typedef struct Light {
        cl_float4 position;
        cl_float4 direction;
        cl_float4 intensity;
    } Light;

    typedef struct MeshLight {
        int i0;
        int i1;
        int i2;
        float area;
        cl_float4 intensity;
    };

    typedef struct LightTreeNode {
        cl_float4 pmin;
        cl_float4 pmax;
        cl_float4 axis;
        float theta_o;
        float theta_e;
        int left;
        int right;
    } LightTreeNode;

#if defined(APP_LSIS)

    inline Vertex make_vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv) {
        Vertex vertex = {};
        vertex.position = { position.x, position.y, position.z, uv.x };
        vertex.normal = { normal.x, normal.y, normal.z, uv.y };
        return vertex;
    }

    inline Face make_face(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t material_index) {
        Face face = {};
        face.index = { v0,v1,v2,material_index };
        return face;
    }

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
        material.emission = { 0.0f,0.0f,0.0f,0.0f };
        return material;
    }

    inline Material make_material(glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emission) {
        Material material = {};
        material.diffuse = { diffuse.x, diffuse.y, diffuse.z, 1.0f };
        material.specular = { specular.x, specular.y, specular.z, 1.0f };
        material.emission = { emission.x, emission.y, emission.z, 1.0f };
        return material;
    }

    inline LightTreeNode make_light_tree_node(glm::vec3 pmin, glm::vec3 pmax, glm::vec3 axis, float theta_o, float theta_e, int left, int right) {
        LightTreeNode node = {};
        node.pmin = { pmin.x, pmin.y, pmin.z, 0.0f };
        node.pmax = { pmax.x, pmax.y, pmax.z, 0.0f };
        node.axis = { axis.x, axis.y, axis.z, 0.0f };
        node.theta_o = theta_o;
        node.theta_e = theta_e;
        node.left = left;
        node.right = right;
        return node;
    }

}
#endif

#endif // !SHARED_DEFINES