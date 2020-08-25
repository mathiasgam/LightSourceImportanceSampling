#include "commonCL.h"

__kernel void generate_rays(
    IN_VAL(uint, width),
    IN_VAL(uint, height),
    IN_VAL(uint, samples),
    IN_VAL(uint, seed),
    IN_VAL(mat4, camera_matrix),
    OUT_BUF(Ray, rays))
{
    const int id = get_global_id(0);
    const int N = width * height * samples;

    uint rng = hash2(hash2(id) ^ hash1(seed));

    if (id == 0){
        //printf("size: [%d,%d], rays: %d, samples: %d, seed: %d\n", width, height, N, samples, seed);
    }

    //barrier(CLK_GLOBAL_MEM_FENCE);
    if (id < N) {

        // project pixels coordinates into unit cube
        float2 pixel_coord = (float2)((float)(id % width), (float)(id / width));
        float2 screen_size = (float2)(width, height);
        float2 jitter = (float2)(rand(&rng), rand(&rng)) - 0.5f; // random number in the range [-0.5,0.5]

        float2 screen_pos = ((pixel_coord + jitter) / screen_size) * 2.0f - 1.0f;
        //float2 screen_pos = (float2)(x * 2.0f - 1.0, y * 2.0 - 1.0);

        float4 near = (float4)(screen_pos.xy, 0.0, 1.0);
        float4 far = (float4)(screen_pos.xy ,1.0, 1.0);
 
        // project from unitcube to world space using the inverse projection matrix
        near = mul(camera_matrix, near);
        near /= near.w;
        far = mul(camera_matrix, far);
        far /= far.w;

        float3 pos = near.xyz;
        float3 dir = (far - near).xyz;

        // Save the ray
        rays[id] = CreateRay(pos.xyz, dir.xyz, 0.001, 1000);
    }
}