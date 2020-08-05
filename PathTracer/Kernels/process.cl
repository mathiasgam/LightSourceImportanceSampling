#include "commonCL.h"

inline float4 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return (float4)(col.xyz, 1.0f);
}

__kernel void process_intersections(
    IN_VAL(uint, width),
    IN_VAL(uint, height),
    IN_VAL(uint, num_rays),
    IN_VAL(uint, num_vertices),
    IN_VAL(uint, num_faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Face, faces),
    IN_BUF(Ray, rays),
    IN_BUF(Intersection, hits),
    OUT_BUF(Ray, rays_out),
    OUT_BUF(Intersection, hits_out),
    OUT_BUF(Pixel, pixels)
){
    int id = get_global_id(0);
    int num_pixels = width * height;

    int center_id = (width / 2) + (height / 2) * width;

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        float x = (float)(id % width);
        float y = (float)(id / width);

        x /= width;
        y /= height;

        float3 pos = ray.origin.xyz;
        float3 dir = ray.direction.xyz;

        float3 UP = (float3)(0,1,0);

        float d = max(dot(dir, UP), 0.0f);

        float3 rgb = (float3)(0.58, 0.92, 1.0);
        float4 color = (float4)(rgb.xyz * d, 0.2);

        float depth = (float)(hit.padding0);
        depth = depth / 24.0;
        depth = 1 - (depth * depth);

        if (hit.hit > 0){
            color = (float4)(depth,depth,depth,0.2);
            float3 hit_pos = ray.origin.xyz + (ray.direction.xyz * hit.uvwt.z);
            hit_pos *= 0.2f;
            hit_pos += 0.5f;
            color = (float4)(hit_pos.xyz, 0.2f);
        }
        

        Pixel p = {};
        p.color = color;
        pixels[hit.pixel_index] = p;
    }else{
        printf("Fail!\n");
    }
    

}