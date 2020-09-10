#include "commonCL.h"

inline float3 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return col.xyz;
}

inline float3 ColorFromPosition(float3 position){
    float3 remain = remainder(position, 10.0f) * 0.1f;
    return (float3)(remain.xyz);
}

inline float3 GetBackground(float3 dir){
    const float3 UP = (float3)(0,1,0);
    const float d = 1.0f - max(dot(dir, UP), 0.0f);

    const float3 sky = (float3)(0.8f, 1.0f, 1.0f);
    const float3 ground = (float3)(0.55f, 0.40f, 0.17f);

    if (d == 1.0f){
        return (float3)(0.2f,0.2f,0.2f);
    }

    return mix(sky, ground, d*d);
}

float2 direction_to_hdri(float3 d){
    float x = (atan(d.x / d.z) + M_PI_F) / (M_PI_F * 2.0f);
    float y = (asin(d.y) + (M_PI_F / 2.0f)) / M_PI_F;
    return (float2)(x,y);
}

const sampler_t sampler_in = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

__kernel void ProcessBounce(
    IN_VAL(uint, num_samples),
    IN_VAL(uint, num_lights),
    IN_VAL(uint, num_pixels),
    IN_VAL(uint, multi_sample_count),
    IN_BUF(Intersection, hits),
    IN_BUF(GeometricInfo, geometrics),
    IN_BUF(Light, lights),
    IN_BUF(Material, materials),
    OUT_BUF(float3, results),
    OUT_BUF(float3, throughputs),
    OUT_BUF(int, states),
    OUT_BUF(float3, light_contribution),
    OUT_BUF(Ray, bounce_rays),
    OUT_BUF(Ray, shadow_rays),
    __read_only image2d_t texture
){
    int id = get_global_id(0);
    uint rng = hash2(hash2(id) ^ hash1(multi_sample_count));

    if (id < num_samples){
        GeometricInfo geometric = geometrics[id];
        Intersection hit = hits[id];

        // choose light
        uint i = random_uint(&rng, num_lights);
        float pdf = inverse(num_lights);

        float3 result = results[id];
        float3 throughput = throughputs[id];

        // iterate over all lights
        //for (uint i = 0; i < num_lights; i++){
        if (states[id] == STATE_ACTIVE) {
            // process miss
            if (hit.hit == 0){
                float2 coord = direction_to_hdri(geometric.incoming.xyz);
                coord *= 10.0f;
                result += read_imagef(texture, sampler_in, coord).xyz;
                //result += GetBackground(geometric.incoming.xyz) * throughput;
                states[id] = STATE_INACTIVE;
            }else{
                //throughput *= max(-dot(geometric.incoming.xyz,geometric.normal.xyz),0.0f); 
                Light light = lights[i];
                Material material = materials[hit.material_index];
                float3 diffuse = material.diffuse.xyz;

                throughput *= diffuse * M_1_PI_F;

                const float3 diff = light.position.xyz - geometric.position.xyz;
                const float dist = length(diff);
                const float dist_inv = inverse(dist);
                const float3 dir = diff * dist_inv;

                float d = dot(geometric.normal.xyz, dir);

                // calculate the lights contribution
                float3 L = light.intensity.xyz * max(d, 0.0f) * (dist_inv * dist_inv);
                //result += L * material.diffuse.xyz * throughput;
                //sample.result.xyz += L * throughput * material.diffuse.xyz;

                float3 lift = geometric.normal.xyz * 0.0001f;

                shadow_rays[id] = CreateRay(geometric.position.xyz + lift, dir, 0.0001f, dist);
                light_contribution[id] = (L * throughput) / pdf;

                float3 out_dir = sample_hemisphere_cosine(&rng, geometric.normal.xyz);
                bounce_rays[id] = CreateRay(geometric.position.xyz + lift, out_dir, 0.0001f, 1000.0f);
            }
        }

        results[id] = result;
        throughputs[id] = throughput;
    }
}

__kernel void shade_occlusion(
    IN_BUF(int, hits),
    IN_BUF(int, states),
    IN_BUF(float3, contributions),
    IN_VAL(uint, num_samples),
    OUT_BUF(float3, results)
){
    const int id = get_global_id(0);

    if (id < num_samples){

        const int hit = hits[id];
        const int state = states[id];
        const float3 result = results[id];
        const float3 contribution = contributions[id];

        if (hit == -1 && state == STATE_ACTIVE){
            results[id] = result + contribution;
        }
    }
}