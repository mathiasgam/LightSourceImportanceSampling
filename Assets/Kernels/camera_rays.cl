#include "commonCL.h"

__kernel void generate_rays(
    IN_VAL(uint, width),
    IN_VAL(uint, height),
    IN_VAL(uint, samples),
    IN_VAL(uint, seed),
    OUT_BUF(Ray, rays),
    OUT_BUF(Intersection, hits))
{
    const int id = get_global_id(0);
    const int N = width * height * samples;

    if (id == 0){
        //printf("size: [%d,%d], rays: %d, samples: %d, seed: %d\n", width, height, N, samples, seed);
    }

    //barrier(CLK_GLOBAL_MEM_FENCE);
    if (id < N) {
        Ray ray = {};
        Intersection hit = {};

        float x = (float)(id % width);
        float y = (float)(id / width);

        x /= width;
        y /= height;

        float2 screen_pos = (float2)(x * 2.0f - 1.0, y * 2.0 - 1.0);

        float4 pos = (float4)(screen_pos.x, screen_pos.y, 0, 1);
        float3 dir = normalize((float3)(screen_pos.xy ,1.0));

        ray = CreateRay(pos.xyz, dir.xyz, 0.001, 1000);
        rays[id] = ray;

        // uv - hit barycentrics, w - ray distance
        float4 uvwt;
        hit.hit = 0;
        hit.primid = -1;
        hit.pixel_index = id;
        hit.padding0 = 0;
        hits[id] = hit;
    }
}