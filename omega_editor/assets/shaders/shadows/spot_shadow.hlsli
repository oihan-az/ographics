#ifndef OMEGA_SPOT_SHADOW_HLSLI
#define OMEGA_SPOT_SHADOW_HLSLI

#include "shadow_map_2D.hlsli"

struct SpotShadow
{
    float4x4 view_projection;
    float4 parameters;
};

float sample_spot_shadow(
    Texture2D<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    SpotShadow shadow,
    float3 world_position,
    float3 normal)
{
    return sample_shadow_2d(
        shadow_map,
        shadow_sampler,
        shadow.view_projection,
        world_position,
        normal,
        shadow.parameters.x,
        shadow.parameters.y
    );
}

#endif // OMEGA_SPOT_SHADOW_HLSLI