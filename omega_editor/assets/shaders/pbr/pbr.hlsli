#ifndef OMEGA_PBR_HLSLI
#define OMEGA_PBR_HLSLI

#include "pbr_surface.hlsli"
#include "brdf.hlsli"

#include "../lights/lighting.hlsli"

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
    uint spot_light_count)
{
    float3 result = 0.0f;

    for (uint i = 0; i < directional_light_count; ++i)
    {
        LightSample light = sample_directional_light(directional_lights[i]);
        result += evaluate_pbr_light(surface, light);
    }

    for (uint i = 0; i < point_light_count; ++i)
    {
        LightSample light = sample_point_light(point_lights[i],surface.position);
        result += evaluate_pbr_light(surface, light);
    }

    for (uint i = 0; i < spot_light_count; ++i)
    {
        LightSample light = sample_spot_light(spot_lights[i],surface.position);
        result += evaluate_pbr_light(surface, light);
    }

    return result;
}

#endif // OMEGA_PBR_HLSLI