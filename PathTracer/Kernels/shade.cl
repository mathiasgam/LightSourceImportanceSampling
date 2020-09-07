#include "commonCL.h"

inline float3 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return col.xyz;
}

inline float3 ColorFromPosition(float3 position){
    float3 remain = remainder(position, 10.0f) / 10.0f;
    return (float3)(remain.xyz);
}

inline float3 GetBackground(float3 dir){
    const float3 UP = (float3)(0,1,0);
    float d = 1.0f - max(dot(dir, UP), 0.0f);

    float3 sky = (float3)(0.58f, 0.92f, 1.0f);
    float3 ground = (float3)(0.55f, 0.40f, 0.17f);

    if (d == 1.0f){
        return (float3)(0.2f,0.2f,0.2f);
    }

    return mix(sky, ground, d*d);
}

__kernel void ProcessBounce(
    IN_VAL(uint, num_samples),
    IN_VAL(uint, num_lights),
    IN_VAL(uint, num_pixels),
    IN_VAL(uint, multi_sample_count),
    IN_BUF(Sample, samples),
    IN_BUF(Light, lights),
    IN_BUF(Material, materials),
    OUT_BUF(float3, results),
    OUT_BUF(float3, throughputs),
    OUT_BUF(int, states),
    OUT_BUF(Ray, bounce_rays),
    OUT_BUF(Ray, shadow_rays),
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

        float3 result = results[id];
        float3 throughput = throughputs[id];

        // iterate over all lights
        //for (uint i = 0; i < num_lights; i++){
        if (states[id] == STATE_ACTIVE) {
            // process miss
            if (sample.prim_id == -1){
                result += GetBackground(sample.incoming.xyz) * throughput;
                states[id] = STATE_INACTIVE;
            }else{
                throughput *= max(-dot(sample.incoming.xyz,sample.normal.xyz),0.0f); 
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
                //result += L * material.diffuse.xyz * throughput;
                //sample.result.xyz += L * throughput * material.diffuse.xyz;
                Ray occlusion_ray = CreateRay(sample.position.xyz, dir, 0.0001f, dist);

                float3 out_dir = sample_hemisphere(&rng, sample.normal.xyz);
                bounce_rays[id] = CreateRay(sample.position.xyz, out_dir, 0.0001f, 1000.0f);
                throughput *= material.diffuse.xyz * max(dot(out_dir, sample.normal.xyz),0.0f);
            }
        }

        results[id] = result;
        float4 color = (float4)(result.xyz, 1.0f);

        float f = 1.0f / (multi_sample_count + 1);
        Pixel p = pixels[id];
        float4 current = p.color;
        p.color = mix(current, color, f);
        //p.color = (float4)(sample.result.xyz, 1.0f);
        pixels[id] = p;

    }
}