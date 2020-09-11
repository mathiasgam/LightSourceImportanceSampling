#include "commonCL.h"

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
        float3 hdrColor = pixel.xyz;
  
        // reinhard tone mapping
        float3 mapped = hdrColor / (hdrColor + 1.0f);
        // gamma correction 
        mapped = pow(mapped, (1.0 / gamma));

        pixel.xyz = mapped;
        pixel = clamp(pixel, 0.0f, 1.0f);
        int2 pos = (int2)(x, y);

        write_imagef(image_out, pos, pixel);
    }
}