#include "commonCL.h"

inline float3 color_correction(const float3 color, const float gamma, const float exposure) {
    // reinhard tone mapping
    const float3 mapped = color / (color + exposure);
    // gamma correction 
    return pow(mapped, 1.0f / gamma);
}

__kernel void write_pixels(
    IN_BUF(float4, input),
    IN_VAL(int2, dim),
    __write_only image2d_t image_out)
{
    const int id = get_global_id(0); 
    const int N = dim.x * dim.y;

    if (id < N) {
        float4 pixel = input[id].xyzw;

        int x = id % dim.x;
        int y = id / dim.x;

        const float gamma = 2.2;
        const float exposure = 1.0f;

        pixel.xyz = color_correction(pixel.xyz, gamma, exposure);
        pixel = clamp(pixel, 0.0f, 1.0f);
        int2 pos = (int2)(x, y);

        write_imagef(image_out, pos, pixel);
    }
}