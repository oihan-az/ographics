#ifndef OMEGA_IRRADIANCE_MAP_GEN_COMP_HLSL
#define OMEGA_IRRADIANCE_MAP_GEN_COMP_HLSL

#include "../cubemap/cubemap.hlsli"
#include "importance_sampling.hlsli"

static const uint IRRADIANCE_SAMPLE_COUNT = 256;

[[vk::binding(0, 0)]]
TextureCube<half4> environment_map;

[[vk::binding(1, 0)]]
RWTexture2DArray<half4> irradiance_map;

[[vk::binding(2, 0)]]
SamplerState environment_sampler;

float3 cosine_sample_hemisphere(float2 xi)
{
    float phi = 2.0f * PI * xi.x;

    float r = sqrt(xi.y);

    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = sqrt(max(1.0f - xi.y, 0.0f));

    return float3(x, y, z);
}

float3 sample_hemisphere_direction(float2 xi, float3 N)
{
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);

    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 H = cosine_sample_hemisphere(xi);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float source_mip_level(float NdotL, float source_resolution, uint source_mips)
{
    // Cosine-weighted hemisphere PDF
    float pdf = max(NdotL / PI, 1e-6f);

    float texel_solid_angle = 4.0f * PI / (6.0f * source_resolution * source_resolution);
    float sample_solid_angle = 1.0f / (float(IRRADIANCE_SAMPLE_COUNT) * pdf);

    float lod = 0.5f * log2(sample_solid_angle / texel_solid_angle);
    return clamp(lod, 0.0f, float(source_mips - 1));
}

float3 evaluate_irradiance(float3 N)
{
    uint source_width;
    uint source_height;
    uint source_mips;

    environment_map.GetDimensions(0, source_width, source_height, source_mips);

    float3 irradiance = 0.0f;
    for (uint i = 0; i < IRRADIANCE_SAMPLE_COUNT; ++i)
    {
        float2 xi = hammersley(i, IRRADIANCE_SAMPLE_COUNT);
        float3 L = sample_hemisphere_direction(xi, N);

        float NdotL = saturate(dot(N, L));
        float lod = source_mip_level(NdotL, float(source_width), source_mips);

        irradiance += environment_map.SampleLevel(environment_sampler, L, lod).rgb;
    }

    return irradiance / float(IRRADIANCE_SAMPLE_COUNT);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height, layers;
    irradiance_map.GetDimensions(width, height, layers);
    if (dispatch_id.x >= width || dispatch_id.y >= height || dispatch_id.z >= layers)
        return;

    float2 uv = (float2(dispatch_id.xy) + 0.5f) / float2(width, height);
    uv = uv * 2.0f - 1.0f;

    float3 N = cubemap_direction(dispatch_id.z, uv);
    float3 irradiance = evaluate_irradiance(N);

    irradiance_map[dispatch_id] = float4(irradiance, 1.0f);
}

#endif // OMEGA_IRRADIANCE_MAP_GEN_COMP_HLSL