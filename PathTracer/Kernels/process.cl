#include "commonCL.h"

inline float4 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return (float4)(col.xyz, 1.0f);
}

inline float4 ColorFromUV(float2 uv){
    return (float4)(uv.x, 0.0f, uv.y, 1.0f);
}

inline float3 CalcGeometricNormal(float3 v0, float3 v1, float3 v2){
    float3 e0 = v1 - v0;
    float3 e1 = v2 - v0;

    return normalize(cross(e0,e1));
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

inline float2 InterpolateFloat2(float2 f0, float2 f1, float2 f2, float2 uv){
    return mix(mix(f0, f1, uv.x), f2, uv.y);
}

inline float3 InterpolateFloat3(float3 f0, float3 f1, float3 f2, float2 uv){
    return mix(mix(f0, f1, uv.x), f2, uv.y);
}

__kernel void process_intersections(
    IN_VAL(uint, width),
    IN_VAL(uint, height),
    IN_VAL(uint, num_samples),
    IN_VAL(uint, num_rays),
    IN_VAL(uint, num_vertices),
    IN_VAL(uint, num_faces),
    IN_BUF(Vertex, vertices),
    IN_BUF(Face, faces),
    IN_BUF(Ray, rays),
    IN_BUF(Intersection, hits),
    OUT_BUF(Ray, rays_out),
    OUT_BUF(Pixel, pixels)
){
    int id = get_global_id(0);
    int num_pixels = width * height;

    int center_id = (width / 2) + (height / 2) * width;

    uint rng = hash2(hash2(id) ^ hash1(num_samples));

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        float3 pos = ray.origin.xyz;
        float3 dir = ray.direction.xyz;

        float4 color;


        if (hit.hit > 0){
            const float3 hit_pos = ray.origin.xyz + (ray.direction.xyz * hit.uvwt.z);

            const Face face = faces[hit.primid];
            const Vertex v0 = vertices[face.index.x];
            const Vertex v1 = vertices[face.index.y];
            const Vertex v2 = vertices[face.index.z];

            float2 uv = hit.uvwt.xy;
            const float3 normal_shading = InterpolateFloat3(GetVertexNormal(v0), GetVertexNormal(v1), GetVertexNormal(v2), uv);
            const float2 tex_coord = InterpolateFloat2(GetVertexUV(v0),GetVertexUV(v1),GetVertexUV(v2),uv);


            color = ColorFromNormal(normal_shading);
            //color = ColorFromUV(tex_coord);
            //color = (float4)(hit.uvwt.xy, 0.0f, 1.0f);
        }else{
            color = GetBackground(dir);
        }
        
        float f = 1.0f / (num_samples + 1);
        Pixel p = pixels[id];
        float4 current = p.color;
        p.color = mix(current, color, f);
        pixels[id] = p;
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