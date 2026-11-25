#ifndef OMEGA_DITHERING_COMP_HLSL
#define OMEGA_DITHERING_COMP_HLSL

#include "../post_processing/dithering.hlsli"

[[vk::binding(0, 0)]]
Texture2D<half4> input_image;

[[vk::binding(1, 0)]]
[[vk::image_format("rgba8")]]
RWTexture2D<unorm float4> output_image;

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height;
    output_image.GetDimensions(width, height);
    if (dispatch_id.x >= width || dispatch_id.y >= height)
        return;

    uint2 pixel = dispatch_id.xy;

    float3 color = input_image.Load(int3(pixel, 0)).rgb;
    color = saturate(color);
    color = dither_8bit(color, float2(pixel));

    output_image[pixel] = float4(color, 1.0f);
}

#endif // OMEGA_DITHERING_COMP_HLSL
