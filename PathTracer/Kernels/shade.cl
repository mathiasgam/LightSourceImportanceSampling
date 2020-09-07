#include "commonCL.h"

inline float4 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return (float4)(col.xyz, 1.0f);
}

__kernel void shade(
    IN_VAL(uint, num_samples),
    IN_VAL(uint, num_lights),
    IN_VAL(uint, num_pixels),
    IN_VAL(uint, multi_sample_count),
    IN_BUF(Sample, samples),
    IN_BUF(Light, lights),
    IN_BUF(Material, materials),
    OUT_BUF(Pixel, pixels)
){
    int id = get_global_id(0);

    uint rng = hash2(hash2(id) ^ hash1(multi_sample_count));

/*
    if (id == 0){
        printf("num_samples: %d, num_lights: %d, num_pixels: %d\n", num_samples, num_lights, num_pixels);
    }
*/
    if (id < num_samples){
        Sample sample = samples[id];

        // choose light
        uint i = random_uint(&rng, num_lights);
        float pdf = 1.0f / num_lights;

        float3 throughput = sample.throughput.xyz;


        // iterate over all lights
        //for (uint i = 0; i < num_lights; i++){
        if (sample.is_active) {
            Light light = lights[i];
            //Material material = materials[sample.material_index];
            Material material = {};
            material.diffuse = (float4)(.8f,.8f,.8f,1.0f);
            material.specular = (float4)(.8f,.8f,.8f,1.0f);

            const float3 diff = light.position.xyz - sample.position.xyz;
            const float dist = length(diff);
            const float dist_inv = 1.0f / dist;
            const float3 dir = diff * dist_inv;

            float d = dot(sample.normal.xyz, dir);

            // calculate the lights contribution
            float3 L = light.intensity.xyz * max(d, 0.0f) * (dist_inv * dist_inv);

            sample.result.xyz += L * throughput * material.diffuse.xyz;
        }

        float4 color = (float4)(sample.result.xyz, 1.0f);

        float f = 1.0f / (multi_sample_count + 1);
        Pixel p = pixels[id];
        float4 current = p.color;
        p.color = mix(current, color, f);
        //p.color = (float4)(sample.result.xyz, 1.0f);
        pixels[id] = p;

    }
}