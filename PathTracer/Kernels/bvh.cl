
#include "commonCL.h"

#define MAX_DEPTH 24

inline float min_component(float3 f)
{
    return min(min(f.x, f.y), f.z);
}

inline float max_component(float3 f){
    return max(max(f.x, f.y), f.z);
}


inline float3 safe_invdir(float3 dir)
{
    float const dirx = dir.x;
    float const diry = dir.y;
    float const dirz = dir.z;
    float const ooeps = 1e-12;
    float3 invdir;
    invdir.x = 1.0f / (fabs(dirx) > ooeps ? dirx : copysign(ooeps, dirx));
    invdir.y = 1.0f / (fabs(diry) > ooeps ? diry : copysign(ooeps, diry));
    invdir.z = 1.0f / (fabs(dirz) > ooeps ? dirz : copysign(ooeps, dirz));
    return invdir;
}

/**
invdir = 1.0f / dir
oxinvdir = -ray.origin * invdir
t_bounds = (t_min, t_max)
 */
inline bool intersect_bbox(float3 pmin, float3 pmax, float3 oxinvdir, float3 invdir, float t_min, float t_max){
    float3 t1 = mad(pmin, invdir, oxinvdir);
    float3 t2 = mad(pmax, invdir, oxinvdir);

    float tmin = max_component(min(t1, t2));
    float tmax = min_component(max(t1, t2));

    tmin = max(tmin, t_min);
    tmax = min(tmax, t_max);

    return tmax >= tmin;
}

inline float intersect_triangle(
    const Ray ray,
    const float3 v1,
    const float3 v2,
    const float3 v3
    )
{
    float3 const e1 = v2 - v1;
    float3 const e2 = v3 - v1;
    float3 const s1 = cross(ray.direction.xyz, e2);
    float const t_max = ray.direction.w;

    float const invd = 1.f / dot(s1, e1);

    float3 const d = ray.origin.xyz - v1;
    float const b1 = dot(d, s1) * invd;
    float3 const s2 = cross(d, e1);
    float const b2 = dot(ray.direction.xyz, s2) * invd;
    float const temp = dot(e2, s2) * invd;

    bool hasHit = b1 < 0.f || b1 > 1.f || b2 < 0.f || b1 + b2 > 1.f || temp < 0.f || temp > t_max;
    return hasHit ? t_max : temp;
}

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
    const int id = get_global_id(0);

    // fixed size queue, for storing the nodes not yet taken when iterating through the tree
    int queue[MAX_DEPTH];

    if (id < num_rays) {
        Ray ray = rays[id];
        Intersection intersection = intersections[id];

        float t_max = ray.direction.w;
        float t_min = ray.origin.w;

        int hits = 0;
        int prim_id = -1;
        float3 normal;

        const float3 invdir = safe_invdir(ray.direction.xyz);
        const float3 origin = ray.origin.xyz;
        const float3 oxinvdir = -origin * invdir;

        int count = 0;
        int next = 0;
        Node node;

        int depth = 0;

        while (next != -1) {
            node = nodes[next];
            int left = (int)node.min.w;
            int right = (int)node.max.w;
            float3 pmin = node.min.xyz;
            float3 pmax = node.max.xyz;

            // Make sure all threads are ready
            barrier(CLK_LOCAL_MEM_FENCE);

            // Intersects bbox
            if (intersect_bbox(pmin, pmax, oxinvdir, invdir, t_min, t_max)) {
                if (left == -1) { // node is a leaf

                    // Fetch the vertices of the triangle
                    Face face = faces[right];
                    Vertex v0 = vertices[face.index.x];
                    Vertex v1 = vertices[face.index.y];
                    Vertex v2 = vertices[face.index.z];

                    // Check if the ray hit the contained triangle and store the distance in f if hit
                    float f = intersect_triangle(ray, v0.position.xyz, v1.position.xyz, v2.position.xyz);

                    // if the hit is closer than the currently closest hit
                    if (f < t_max) {
                        t_max = f;
                        prim_id = right;
                        hits += 1;
                    }

                    // get the next node from the queue if any is left
                    next = count > 0 ? queue[--count] : -1;
                }
                else { // node is internal
                    next = left;
                    queue[count++] = right;
                }
            }
            else {
                // get the next node from the queue
                next = count > 0 ? queue[--count] : -1;
            }
            depth = max(count, depth);
        }
        
        // save intersection info
        intersection.hit = hits;
        intersection.primid = prim_id;
        intersection.padding0 = depth;
        intersection.uvwt = (float4)(0.5f, 0.5f, t_max, 0.0f);
        intersections[id] = intersection;
    }

}