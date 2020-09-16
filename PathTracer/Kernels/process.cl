#include "commonCL.h"

inline float3 CalcGeometricNormal(float3 v0, float3 v1, float3 v2){
    float3 e0 = v1 - v0;
    float3 e1 = v2 - v0;

    return normalize(cross(e0,e1));
}

inline float2 InterpolateFloat2(float2 f0, float2 f1, float2 f2, float2 uv){
    return mix(mix(f0, f1, uv.x), f2, uv.y);
}

inline float3 InterpolateFloat3(float3 f0, float3 f1, float3 f2, float2 uv){
    return mix(mix(f0, f1, uv.x), f2, uv.y);
}

__kernel void process_intersections(
    IN_VAL(uint, multi_sample_count),
    IN_VAL(uint, num_rays),
    IN_VAL(uint, num_vertices),
    IN_VAL(uint, num_faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Face, faces),
    IN_BUF(Ray, rays),
    IN_BUF(Intersection, hits),
    IN_BUF(int, states),
    OUT_BUF(GeometricInfo, geometrics)
){
    int id = get_global_id(0);

    uint rng = hash2(hash2(id) ^ hash1(multi_sample_count));

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        GeometricInfo geometric = geometrics[id];

        /*
        if (hit.hit > 0 && states[id] == STATE_ACTIVE){

            const float3 hit_pos = hit.position.xyz;
            const float3 normal = hit.normal.xyz;
            const float3 lift = normal * 0.0001f;
            const float3 incoming = hit.incoming.xyz;

            geometrics.position = (float4)(hit_pos + lift, 0.0f);
            geometrics.normal = (float4)(normal, 0.0f);
            geometrics.incoming = (float4)(incoming, 0.0f);

        }else{
            sample.position = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.normal = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.incoming = (float4)(ray.direction.xyz, 0.0f);
            sample.material_index = -1;
            sample.prim_id = -1;
        }
        samples[id] = sample;
        */
    }
    
}

__kernel void process_light_sample(
    IN_VAL(uint, num_rays),
    IN_VAL(uint, num_samples),
    IN_BUF(Ray, rays),
    IN_BUF(Intersection, hits),
    OUT_BUF(Pixel, pixels)
) {
    const uint id = get_global_id(0);

    if (id < num_rays){
        Intersection hit = hits[id];

        if (hit.hit == 0){
            uint pixel_index = id;
            Pixel p = pixels[pixel_index];
            p.color += 0.1f;
            pixels[pixel_index] = p;
        }

    }

}

__kernel void process_results(
    IN_BUF(float3, results),
    IN_VAL(uint, N),
    IN_VAL(uint, sample_count),
    OUT_BUF(Pixel, pixels)
){
    const int id = get_global_id(0);

    if (id < N){
        float3 result = results[id];
        float3 current = pixels[id].color.xyz;

        //float f = sample_count * inverse(sample_count + 1);
        //pixels[id].color = (float4)(mix(result, current, f), 1.0f);

        // if more samples is present, mix with current sample
        if (sample_count > 0){
            result = ((current * sample_count) + result) / (sample_count+1);
        }
        // save result in the pixel buffer
        pixels[id].color = (float4)(result.xyz, 1.0f);
    }
}