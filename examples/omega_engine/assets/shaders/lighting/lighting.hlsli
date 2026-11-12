#ifndef OMEGA_LIGHTING_HLSLI
#define OMEGA_LIGHTING_HLSLI

#include "directional_light.hlsli"
#include "point_light.hlsli"
#include "spot_light.hlsli"

struct LightingSurface
{
    float3 position;
    float3 normal;
};

float3 evaluate_lighting(
    LightingSurface surface,
    StructuredBuffer<DirectionalLight> directional_lights,
    uint directional_light_count,
    StructuredBuffer<PointLight> point_lights,
    uint point_light_count,
    StructuredBuffer<SpotLight> spot_lights,
    uint spot_light_count)
{
    float3 normal = normalize(surface.normal);
    float3 result = 0.0;

    for (uint i = 0; i < directional_light_count; ++i)
        result += evaluate_directional_light(directional_lights[i], normal);

    for (uint i = 0; i < point_light_count; ++i)
        result += evaluate_point_light(point_lights[i], surface.position, normal);

    for (uint i = 0; i < spot_light_count; ++i)
        result += evaluate_spot_light(spot_lights[i], surface.position, normal);

    return result;
}

#endif // OMEGA_LIGHTING_HLSLI