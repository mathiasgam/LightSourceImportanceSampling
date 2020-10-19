
#include "commonCL.h"

#define MAX_DEPTH 24

#define min_component(vec) min3(vec.x, vec.y, vec.z)
#define max_component(vec) max3(vec.x, vec.y, vec.z)

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

inline float2 fast_intersect_bbox(AABB bbox, float3 oxinvdir, float3 invdir, float t_min, float t_max){
    float3 t1 = mad(bbox.min.xyz, invdir, oxinvdir);
    float3 t2 = mad(bbox.max.xyz, invdir, oxinvdir);

    float tmin = max_component(min(t1,t2));
    float tmax = min_component(max(t1,t2));

    tmin = max(tmin, t_min);
    tmax = min(tmax, t_max);

    return (float2)(tmin, tmax);
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
    
    float const invd = inverse(dot(s1, e1));
    if (invd <= 0.0f)
        return t_max;

    float3 const d = ray.origin.xyz - v1;
    float const b1 = dot(d, s1) * invd;
    float3 const s2 = cross(d, e1);
    float const b2 = dot(ray.direction.xyz, s2) * invd;
    float const temp = dot(e2, s2) * invd;

    bool hasHit = b1 < 0.f || b1 > 1.f || b2 < 0.f || b1 + b2 > 1.f || temp < 0.f || temp > t_max;
    return hasHit ? t_max : temp;
}

inline float2 calculate_triangle_barycentrics(float3 p, float3 v0, float3 v1, float3 v2) {
    float3 const e1 = v1 - v0;
    float3 const e2 = v2 - v0;
    float3 const e = p - v0;
    float const d00 = dot(e1, e1);
    float const d01 = dot(e1, e2);
    float const d11 = dot(e2, e2);
    float const d20 = dot(e, e1);
    float const d21 = dot(e, e2);

    float denom = (d00 * d11 - d01 * d01);

    if (denom == 0.f)
    {
        return (float2)(0.0f,0.0f);
    }

    float const invdenom = inverse(denom);

    float const b1 = (d11 * d20 - d01 * d21) * invdenom;
    float const b2 = (d00 * d21 - d01 * d20) * invdenom;

    return (float2)(b1, b2);
}


/**
Based on shortstack bvh2 from RadeonRays SDK 2.0 
Link: "https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK/blob/legacy-2.0/RadeonRays/src/kernels/CL/intersect_bvh2_short_stack.cl"
 */ 
__kernel void intersect_bvh(
    IN_BUF(Node, nodes),
    IN_BUF(AABB, bboxes),
    IN_BUF(Face, faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Ray, rays),
    IN_VAL(int, num_rays),
    OUT_BUF(Intersection, intersections),
    OUT_BUF(GeometricInfo, geometric_info),
    IN_BUF(uint, active_rays)
){
    const int id = get_global_id(0);

    // fixed size queue, for storing the nodes not yet taken when iterating through the tree
    int queue[MAX_DEPTH];

    if (id < active_rays[0]) {
        Ray ray = rays[id];

        float t_max = ray.direction.w;
        float t_min = ray.origin.w;

        int hits = 0;
        int prim_id = -1;

        const float3 invdir = safe_invdir(ray.direction.xyz);
        const float3 origin = ray.origin.xyz;
        const float3 oxinvdir = -origin * invdir;

        int count = 0;
        int next = 0;
        Node node;

        int depth = 0;

        while (next != -1) {
            node = nodes[next];
            //bbox = bboxes[next];
            int left = node.left;
            int right = node.right;

            // If node is leaf
            if (left == -1){
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

            }else{ // Node is internal
                const AABB bbox_l = bboxes[left];
                const AABB bbox_r = bboxes[right];

                // test intersection for both childnodes
                const float2 s0 = fast_intersect_bbox(bbox_l, oxinvdir, invdir, t_min, t_max);
                const float2 s1 = fast_intersect_bbox(bbox_r, oxinvdir, invdir, t_min, t_max);

                const bool traverse_left = (s0.x <= s0.y);
                const bool traverse_right = (s1.x <= s1.y);
                const bool right_first = traverse_right && (s0.x > s1.x);

                if (traverse_left || traverse_right){
                    int deffered = -1;

                    if (right_first || !traverse_left){
                        next = right;
                        deffered = left;
                    }else{
                        next = left;
                        deffered = right;
                    }

                    if (traverse_left && traverse_right){
                        queue[count++] = deffered;
                    }

                    continue;
                }
            }

            // get the next node from the queue
            next = count > 0 ? queue[--count] : -1;

            // Make sure all threads are ready
            //barrier(CLK_LOCAL_MEM_FENCE);

            depth = max(count, depth);
        }

        Intersection hit = {};
        GeometricInfo info = {};
        if (hits) {
            const Face face = faces[prim_id];
            const Vertex v0 = vertices[face.index.x];
            const Vertex v1 = vertices[face.index.y];
            const Vertex v2 = vertices[face.index.z];

            const float3 hit_pos = ray.origin.xyz + ray.direction.xyz * t_max;
            const float2 uv = calculate_triangle_barycentrics(hit_pos, v0.position.xyz, v1.position.xyz, v2.position.xyz);
            const float3 normal_shading = interpolate(GetVertexNormal(v0), GetVertexNormal(v1), GetVertexNormal(v2), uv);
            const float2 tex_coord = interpolate(GetVertexUV(v0),GetVertexUV(v1),GetVertexUV(v2),uv);

            const float flip = dot(ray.direction.xyz, normal_shading) < 0.0f ? 1.0f : -1.0f;

            hit.material_index = face.index.w;
            info.position = (float4)(hit_pos, 0.0f);
            info.normal = (float4)(normal_shading * flip, 0.0f);
            info.uvwt = (float4)(uv.xy, 0.0f, t_max);
        }else{
            hit.material_index = -1;
        }

        info.incoming = (float4)(ray.direction.xyz, 0.0f);
        
        // save intersection info
        hit.hit = hits;
        hit.prim_index = prim_id;
        intersections[id] = hit;
        geometric_info[id] = info;
    }

}

/**
Based on shortstack bvh2 from RadeonRays SDK 2.0 
Link: "https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK/blob/legacy-2.0/RadeonRays/src/kernels/CL/intersect_bvh2_short_stack.cl"
 */ 
__kernel void occluded(
    IN_BUF(Node, nodes),
    IN_BUF(AABB, bboxes),
    IN_BUF(Face, faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Ray, rays),
    IN_VAL(uint, num_rays),
    OUT_BUF(int, hits),
    IN_BUF(uint, active_rays)
){
    const int id = get_global_id(0);

    // fixed size queue, for storing the nodes not yet taken when iterating through the tree
    int queue[MAX_DEPTH];

    if (id < active_rays[0]){
        // fetch ray data
        const Ray ray = rays[id];

        float t_max = ray.direction.w;
        float t_min = ray.origin.w;

        const float3 invdir = safe_invdir(ray.direction.xyz);
        const float3 origin = ray.origin.xyz;
        const float3 oxinvdir = -origin * invdir;

        int count = 0;
        int next = 0;

        while (next != -1){
            const Node node = nodes[next];

            const int left = node.left;
            const int right = node.right;

            if (left == -1){
                 // Fetch the vertices of the triangle
                Face face = faces[right];
                Vertex v0 = vertices[face.index.x];
                Vertex v1 = vertices[face.index.y];
                Vertex v2 = vertices[face.index.z];

                // Check if the ray hit the contained triangle and store the distance in f if hit
                float f = intersect_triangle(ray, v0.position.xyz, v1.position.xyz, v2.position.xyz);

                // if the
                if (f < t_max) {
                    hits[id] = 1;
                    return;
                }
            }else{
                const AABB bbox_l = bboxes[left];
                const AABB bbox_r = bboxes[right];

                // test intersection for both childnodes
                const float2 s0 = fast_intersect_bbox(bbox_l, oxinvdir, invdir, t_min, t_max);
                const float2 s1 = fast_intersect_bbox(bbox_r, oxinvdir, invdir, t_min, t_max);

                const bool traverse_left = (s0.x <= s0.y);
                const bool traverse_right = (s1.x <= s1.y);
                const bool right_first = traverse_right && (s0.x > s1.x);

                if (traverse_left || traverse_right){
                    int deffered = -1;

                    if (right_first || !traverse_left){
                        next = right;
                        deffered = left;
                    }else{
                        next = left;
                        deffered = right;
                    }

                    if (traverse_left && traverse_right){
                        queue[count++] = deffered;
                    }

                    continue;
                }
            }

            // get the next node from the queue
            next = count > 0 ? queue[--count] : -1;
        }
        hits[id] = -1;
    }
}