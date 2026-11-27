#ifndef OMEGA_DIRECTIONAL_SHADOW_HLSLI
#define OMEGA_DIRECTIONAL_SHADOW_HLSLI

#include "shadow_map_2D.hlsli"
#include "shadow_filter.hlsli"

struct DirectionalShadow
{
    float4x4 view_projection;
    float4 parameters; // depth bias, normal bias, near plane, resolution
    ShadowFilter filter;
    float4 projection_parameters; // near plane, far plane, unused, unused
};

float sample_directional_shadow(
    Texture2D<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    SamplerState shadow_depth_sampler,
    DirectionalShadow shadow,
    float3 world_position,
    float3 normal,
    float2 screen_position)
{
    return sample_shadow_2d(
        shadow_map,
        shadow_sampler,
        shadow_depth_sampler,
        shadow.view_projection,
        world_position,
        normal,
        shadow.parameters.x,
        shadow.parameters.y,
        shadow.projection_parameters.x,
        shadow.projection_parameters.y,
        false,
        shadow.parameters.w,
        shadow.filter,
        screen_position
    );
}

#endif // OMEGA_DIRECTIONAL_SHADOW_HLSLI