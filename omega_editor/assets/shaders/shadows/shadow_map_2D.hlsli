#ifndef OMEGA_SHADOW_MAP_2D_HLSLI
#define OMEGA_SHADOW_MAP_2D_HLSLI

float sample_shadow_2d(
    Texture2D<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    float4x4 view_projection,
    float3 world_position,
    float3 normal,
    float depth_bias,
    float normal_bias)
{
    const float3 sample_position = world_position + normal * normal_bias;
    const float4 clip = mul(view_projection, float4(sample_position, 1.0f));
    if (clip.w <= 0.0f)
        return 1.0f;

    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = ndc.xy * 0.5f + 0.5f;
    if (any(uv < 0.0f) || any(uv > 1.0f) || ndc.z < 0.0f || ndc.z > 1.0f)
    {
        return 1.0f;
    }

    return shadow_map.SampleCmpLevelZero(shadow_sampler, uv, ndc.z - depth_bias);
}

#endif // OMEGA_SHADOW_MAP_2D_HLSLI