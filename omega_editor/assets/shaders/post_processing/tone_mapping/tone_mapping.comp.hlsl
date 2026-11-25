#ifndef OMEGA_TONE_MAPPING_COMP_HLSL
#define OMEGA_TONE_MAPPING_COMP_HLSL

struct ToneMappingConstants
{
    float exposure;
    float3 padding;
};

[[vk::push_constant]]
ConstantBuffer<ToneMappingConstants> constants;

[[vk::binding(0, 0)]]
Texture2D<half4> hdr_image;

[[vk::binding(1, 0)]]
RWTexture2D<half4> output_image;

[[vk::binding(2, 0)]]
SamplerState image_sampler;

float3 tonemap_aces(float3 color)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height;
    output_image.GetDimensions(width, height);
    if (dispatch_id.x >= width || dispatch_id.y >= height)
        return;

    float2 uv = (float2(dispatch_id.xy) + 0.5f) / float2(width, height);
    float4 hdr_color = hdr_image.SampleLevel(image_sampler, uv, 0.0f);

    float3 exposed_color = max(hdr_color.rgb, 0.0f) * constants.exposure;
    float3 mapped_color = tonemap_aces(exposed_color);

    output_image[dispatch_id.xy] = float4(mapped_color, hdr_color.a);
}

#endif // OMEGA_TONE_MAPPING_COMP_HLSL
