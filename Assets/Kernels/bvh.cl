
#include "commonCL.h"

typedef struct Vertex {
    float4 position;
    float4 normal;
    float4 uv;
};

typedef struct Face {
    int4 index;
};

typedef struct Node {
    float4 min; // .w is left child
    float4 max; // .w is right child
};



__kernel void intersect_bvh(
    IN_BUF(Node, nodes),
    IN_BUF(Face, faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Ray, rays),
    IN_VAL(int, num_nodes),
    IN_VAL(int, num_faces),
    IN_VAL(int, num_vertices),
    IN_VAL(int, num_rays),
    OUT_BUF(Intersection, intersections)
){


}