#include "commonCL.h"

inline float4 ColorFromNormal(float3 normal){
    float3 col = normalize(normal).xyz * 0.5f + 0.5f;
    return (float4)(col.xyz, 1.0f);
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
    OUT_BUF(Intersection, hits_out),
    OUT_BUF(Pixel, pixels)
){
    int id = get_global_id(0);
    int num_pixels = width * height;

    int center_id = (width / 2) + (height / 2) * width;

    if (id < num_rays){
        Ray ray = rays[id];
        Intersection hit = hits[id];

        float3 pos = ray.origin.xyz;
        float3 dir = ray.direction.xyz;

        float4 color;


        if (hit.hit > 0){
            //float3 hit_pos = ray.origin.xyz + (ray.direction.xyz * hit.uvwt.z);

            const Face face = faces[hit.primid];
            const Vertex v0 = vertices[face.index.x];
            const Vertex v1 = vertices[face.index.y];
            const Vertex v2 = vertices[face.index.z];

            float u = hit.uvwt.x;
            float v = hit.uvwt.y;
            const float3 normal_shading = mix(mix(v0.normal.xyz, v1.normal.xyz,u), v2.normal.xyz, v);
            const float3 normal_geometric = CalcGeometricNormal(v0.position.xyz, v1.position.xyz, v2.position.xyz);

            color = ColorFromNormal(normal_shading);
            //color = (float4)(hit.uvwt.xy, 0.0f, 1.0f);
        }else{
            color = GetBackground(dir);
        }
        
        float f = 1.0f / (num_samples + 1);
        Pixel p = pixels[hit.pixel_index];
        float4 current = p.color;
        p.color = mix(current, color, f);
        pixels[hit.pixel_index] = p;
    }else{
        printf("Fail!\n");
    }
    

}