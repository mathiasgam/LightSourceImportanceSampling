#include "commonCL.h"

inline float4 ColorFromUV(float2 uv){
    return (float4)(uv.x, 0.0f, uv.y, 1.0f);
}

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

inline float4 GetBackground(float3 dir){
    const float3 UP = (float3)(0,1,0);
    float d = 1.0f - max(dot(dir, UP), 0.0f);

    float3 sky = (float3)(0.58f, 0.92f, 1.0f);
    float3 ground = (float3)(0.55f, 0.40f, 0.17f);

    if (d == 1.0f){
        return (float4)(0.2f,0.2f,0.2f,1.0f);
    }

    return (float4)(mix(sky, ground, d*d),1.0f);
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
    OUT_BUF(Sample, samples)
){
    int id = get_global_id(0);

    uint rng = hash2(hash2(id) ^ hash1(multi_sample_count));

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        float3 pos = ray.origin.xyz;
        float3 dir = ray.direction.xyz;

        Sample sample = {};
        sample.pixel_index = id;


        if (hit.hit > 0){
            const float3 hit_pos = ray.origin.xyz + (ray.direction.xyz * hit.uvwt.z);

            const Face face = faces[hit.primid];
            const Vertex v0 = vertices[face.index.x];
            const Vertex v1 = vertices[face.index.y];
            const Vertex v2 = vertices[face.index.z];

            float2 uv = hit.uvwt.xy;
            const float3 normal_shading = InterpolateFloat3(GetVertexNormal(v0), GetVertexNormal(v1), GetVertexNormal(v2), uv);
            const float2 tex_coord = InterpolateFloat2(GetVertexUV(v0),GetVertexUV(v1),GetVertexUV(v2),uv);


            sample.position = (float4)(hit_pos, 0.0f);
            sample.normal = (float4)(normal_shading, 0.0f);
            sample.incoming = (float4)(ray.direction.xyz, 0.0f);
            sample.throughput = (float4)(1.0f,1.0f,1.0f,1.0f);
            sample.result = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.is_active = 1;
            sample.material_index = face.index.w;
            sample.prim_id = hit.primid;

        }else{
            sample.position = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.normal = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.incoming = (float4)(ray.direction.xyz, 0.0f);
            sample.throughput = (float4)(0.0f,0.0f,0.0f,0.0f);
            sample.result = GetBackground(ray.direction.xyz);
            sample.is_active = 0;
            sample.material_index = -1;
            sample.prim_id = -1;
        }

        samples[id] = sample;

    }else{
        printf("Fail!\n");
    }
    

}

__kernel void process_light_sample(
    IN_VAL(uint, num_rays),
    IN_VAL(uint, num_samples),
    IN_BUF(LightSample, samples),
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