#ifndef OMEGA_ENVIRONMENT_PREFILTER_COMP_HLSL
#define OMEGA_ENVIRONMENT_PREFILTER_COMP_HLSL

#include "../cubemap/cubemap.hlsli"
#include "importance_sampling.hlsli"

struct PrefilterConstants
{
    float roughness;
    uint sample_count;
    uint padding0;
    uint padding1;
};

[[vk::push_constant]]
ConstantBuffer<PrefilterConstants> constants;

[[vk::binding(0, 0)]]
TextureCube<half4> environment_map;

[[vk::binding(1, 0)]]
RWTexture2DArray<half4> prefiltered_map;

[[vk::binding(2, 0)]]
SamplerState environment_sampler;

float3 prefilter_environment(float3 N, float roughness)
{
    float3 V = N;

    if (roughness <= 0.0f)
        return environment_map.SampleLevel(environment_sampler, N, 0.0f).rgb;

    uint source_width, source_height, source_mips;
    environment_map.GetDimensions(0, source_width, source_height, source_mips);

    float3 radiance = 0.0f;
    float total_weight = 0.0f;

    for (uint i = 0; i < constants.sample_count; ++i)
    {
        float2 xi = hammersley(i, constants.sample_count);

        float3 H = importance_sample_ggx(xi, N, roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = saturate(dot(N, L));

        if (NdotL > 0.0f)
        {
            float lod = ggx_sample_lod(N, H, V, roughness, constants.sample_count, float(source_width));
            lod = clamp(lod, 0.0f, float(source_mips - 1));

            float3 sample_radiance = environment_map.SampleLevel(environment_sampler, L, lod).rgb;
            radiance += sample_radiance * NdotL;
            total_weight += NdotL;
        }
    }

    return radiance / max(total_weight, 1e-5f);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height, layers;
    prefiltered_map.GetDimensions(width, height, layers);

    if (dispatch_id.x >= width || dispatch_id.y >= height || dispatch_id.z >= layers)
        return;

    float2 uv = (float2(dispatch_id.xy) + 0.5f) / float2(width, height);
    uv = uv * 2.0f - 1.0f;

    float3 direction = cubemap_direction(dispatch_id.z, uv);
    float3 radiance = prefilter_environment(direction, constants.roughness);

    prefiltered_map[dispatch_id] = float4(radiance, 1.0f);
}

#endif // OMEGA_ENVIRONMENT_PREFILTER_COMP_HLSL