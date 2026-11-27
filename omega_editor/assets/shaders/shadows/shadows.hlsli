#ifndef OMEGA_SHADOWS_HLSLI
#define OMEGA_SHADOWS_HLSLI

#include "shadow_lookup.hlsli"
#include "directional_shadow.hlsli"
#include "point_shadow.hlsli"
#include "spot_shadow.hlsli"

float evaluate_directional_shadow(
    uint light_index,
    float3 world_position,
    float3 normal,
    StructuredBuffer<ShadowLookup> shadow_lookup,
    StructuredBuffer<DirectionalShadow> shadow_data,
    Texture2D<float> shadow_maps[],
    SamplerComparisonState shadow_sampler)
{
    const ShadowLookup lookup = shadow_lookup[light_index];
    if (!has_shadow(lookup))
        return 1.0f;

    const DirectionalShadow shadow = shadow_data[lookup.data_index];

    return sample_directional_shadow(
        shadow_maps[NonUniformResourceIndex(lookup.texture_index)],
        shadow_sampler,
        shadow,
        world_position,
        normal
    );
}

float evaluate_spot_shadow(
    uint light_index,
    float3 world_position,
    float3 normal,
    StructuredBuffer<ShadowLookup> shadow_lookup,
    StructuredBuffer<SpotShadow> shadow_data,
    Texture2D<float> shadow_maps[],
    SamplerComparisonState shadow_sampler)
{
    const ShadowLookup lookup = shadow_lookup[light_index];
    if (!has_shadow(lookup))
        return 1.0f;

    const SpotShadow shadow = shadow_data[lookup.data_index];

    return sample_spot_shadow(
        shadow_maps[NonUniformResourceIndex(lookup.texture_index)],
        shadow_sampler,
        shadow,
        world_position,
        normal
    );
}

float evaluate_point_shadow(
    uint light_index,
    float3 light_position,
    float light_range,
    float3 world_position,
    float3 normal,
    StructuredBuffer<ShadowLookup> shadow_lookup,
    StructuredBuffer<PointShadow> shadow_data,
    TextureCube<float> shadow_maps[],
    SamplerComparisonState shadow_sampler)
{
    const ShadowLookup lookup = shadow_lookup[light_index];
    if (!has_shadow(lookup))
        return 1.0f;

    const PointShadow shadow = shadow_data[lookup.data_index];

    return sample_point_shadow(
        shadow_maps[NonUniformResourceIndex(lookup.texture_index)],
        shadow_sampler,
        shadow,
        light_position,
        light_range,
        world_position,
        normal
    );
}

#endif // OMEGA_SHADOWS_HLSLI