#ifndef SHARED_DEFINES
#define SHARED_DEFINES

#ifdef APP_LSIS // in CPP
// define cl types to match hlsl
#include "CL/cl.h"
#include <cassert>

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
        cl_float4 direction; // w is emission halfangle
        cl_float4 intensity;
        cl_float4 tangent;
        cl_float4 bitangent;
    } Light;

    typedef struct MeshLight {
        int i0;
        int i1;
        int i2;
        float area;
        cl_float4 intensity;
    };

#define LEAF(node) node.type == 0
#define INTERNAL(node) node.type == 1
#define THETA_O(node) node.axis.w
#define THETA_E(node) node.energy.w
#define AXIS(node) node.axis.xyz
#define ENERGY(node) node.energy.xyz
#define INDEX(node) node.left
#define COUNT(node) node.right


    typedef struct LightTreeNode {
        cl_float4 pmin;
        cl_float4 pmax;
        cl_float4 axis; // .w is theta_o
        cl_float4 energy; // .w is theta_e
        int left;
        int right;
        int type;
        int padding;
    } LightTreeNode;

#ifdef APP_LSIS

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
        light.direction = { direction.x, direction.y, direction.z, CL_M_PI };
        light.intensity = { intensity.x, intensity.y, intensity.z, 0.0f };
        light.tangent = { 0,0,0,0 };
        light.bitangent = { 0,0,0,0 };
        return light;
    }

    inline Light make_light(glm::vec3 position, glm::vec3 direction, glm::vec3 intensity, float theta_e) {
        Light light = {};
        light.position = { position.x, position.y, position.z, 0.0f };
        light.direction = { direction.x, direction.y, direction.z, theta_e };
        light.intensity = { intensity.x, intensity.y, intensity.z, 0.0f };
        return light;
    }

    inline Light make_mesh_light(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 normal, glm::vec3 intensity) {
        Light light = {};
        light.position = { p0.x, p0.y, p0.z, 1.0f };
        light.direction = { normal.x, normal.y, normal.z, CL_M_PI_F / 2.0f };
        light.intensity = { intensity.x, intensity.y, intensity.z, 0.0f };

        const glm::vec3 t = p1 - p0;
        const glm::vec3 b = p2 - p0;
        light.tangent = { t.x,t.y,t.z,0 };
        light.bitangent = { b.x,b.y,b.z,0 };
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

    inline LightTreeNode make_light_tree_leaf(glm::vec3 pmin, glm::vec3 pmax, glm::vec3 axis, glm::vec3 energy, float theta_o, float theta_e, int light, int count) {
        LightTreeNode node = {};
        node.pmin = { pmin.x, pmin.y, pmin.z, 0.0f };
        node.pmax = { pmax.x, pmax.y, pmax.z, 0.0f };
        node.axis = { axis.x, axis.y, axis.z, theta_o };
        node.energy = { energy.x, energy.y,energy.x, theta_e };
        node.left = light;
        node.right = count;
        node.type = 0;
        return node;
    }

    inline LightTreeNode make_light_tree_node(glm::vec3 pmin, glm::vec3 pmax, glm::vec3 axis, glm::vec3 energy, float theta_o, float theta_e, int left, int right) {
        LightTreeNode node = {};
        assert(left != -1);
        node.pmin = { pmin.x, pmin.y, pmin.z, 0.0f };
        node.pmax = { pmax.x, pmax.y, pmax.z, 0.0f };
        node.axis = { axis.x, axis.y, axis.z, theta_o };
        node.energy = { energy.x, energy.y,energy.x, theta_e };
        node.left = left;
        node.right = right;
        node.type = 1;
        return node;
    }

}
#endif

#endif // !SHARED_DEFINES