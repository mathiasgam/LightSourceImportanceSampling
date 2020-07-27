#include "commonCL.h"

__kernel void process_intersections(
    IN_VAL(uint, width),
    IN_VAL(uint, height),
    IN_VAL(uint, num_rays),
    IN_BUF(Ray, rays),
    IN_BUF(Intersection, hits),
    OUT_BUF(Pixel, pixels)
){
    int id = get_global_id(0);
    int num_pixels = width * height;

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        float x = (float)(id % width);
        float y = (float)(id / width);

        x /= width;
        y /= height;

        float3 dir = ray.direction.xyz;
        dir = dir * 0.5f + 0.5f;
        float4 color = (float4)(dir,1);

        Pixel p = {};
        p.color = color;
        pixels[id] = p;
    }else{
        printf("Fail!\n");
    }
    

}