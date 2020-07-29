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

        float4 color = (float4)(0.0);

        float depth = (float)(hit.padding0);
        depth = depth / 24.0;
        depth = 1 - (depth * depth);

        if (hit.hit > 0){
            color = (float4)(depth,depth,depth,0.2);
        }
        

        Pixel p = {};
        p.color = color;
        pixels[hit.pixel_index] = p;
    }else{
        printf("Fail!\n");
    }
    

}