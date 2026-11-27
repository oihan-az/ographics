#ifndef OMEGA_PBR_HLSLI
#define OMEGA_PBR_HLSLI

#include "pbr_surface.hlsli"
#include "brdf.hlsli"

#include "../lights/lighting.hlsli"
#include "../shadows/shadows.hlsli"

float3 evaluate_pbr_light(PbrSurface surface, LightSample light)
{
    return evaluate_pbr_brdf(surface, light.direction, light.radiance);
}

float3 evaluate_pbr_lighting(
    PbrSurface surface,
    StructuredBuffer<DirectionalLight> directional_lights,
    uint directional_light_count,
    StructuredBuffer<PointLight> point_lights,
    uint point_light_count,
    StructuredBuffer<SpotLight> spot_lights,
    uint spot_light_count,
    StructuredBuffer<ShadowLookup> directional_shadow_lookup,
    StructuredBuffer<ShadowLookup> spot_shadow_lookup,
    StructuredBuffer<ShadowLookup> point_shadow_lookup,
    StructuredBuffer<DirectionalShadow> directional_shadow_data,
    StructuredBuffer<SpotShadow> spot_shadow_data,
    StructuredBuffer<PointShadow> point_shadow_data,
    Texture2D<float> directional_shadow_maps[],
    Texture2D<float> spot_shadow_maps[],
    TextureCube<float> point_shadow_maps[],
    SamplerComparisonState shadow_sampler,
    SamplerState shadow_depth_sampler,
    float2 screen_position)
{
    float3 result = 0.0f;

    for (uint i = 0; i < directional_light_count; ++i)
    {
        const DirectionalLight light = directional_lights[i];
        const LightSample sample = sample_directional_light(light);

        const float visibility = evaluate_directional_shadow(
            i,
            surface.position,
            surface.normal,
            directional_shadow_lookup,
            directional_shadow_data,
            directional_shadow_maps,
            shadow_sampler,
            shadow_depth_sampler,
            screen_position
        );

        result += evaluate_pbr_light(surface, sample) * visibility;
    }

    for (uint i = 0; i < point_light_count; ++i)
    {
        const PointLight light = point_lights[i];
        const LightSample sample = sample_point_light(light, surface.position);

        const float visibility = evaluate_point_shadow(
            i,
            light.position_range.xyz,
            light.position_range.w,
            surface.position,
            surface.normal,
            point_shadow_lookup,
            point_shadow_data,
            point_shadow_maps,
            shadow_sampler,
            shadow_depth_sampler,
            screen_position
        );

        result += evaluate_pbr_light(surface, sample) * visibility;
    }

    for (uint i = 0; i < spot_light_count; ++i)
    {
        const SpotLight light = spot_lights[i];
        const LightSample sample = sample_spot_light(light, surface.position);

        const float visibility = evaluate_spot_shadow(
            i,
            surface.position,
            surface.normal,
            spot_shadow_lookup,
            spot_shadow_data,
            spot_shadow_maps,
            shadow_sampler,
            shadow_depth_sampler,
            screen_position
        );

        result += evaluate_pbr_light(surface, sample) * visibility;
    }

    return result;
}

#endif // OMEGA_PBR_HLSLI