#ifndef OMEGA_SPOT_LIGHT_HLSLI
#define OMEGA_SPOT_LIGHT_HLSLI

#include "light_sample.hlsli"

struct SpotLight
{
    // XYZ position, W range
    float4 position_range;

    // XYZ direction
    float4 direction;

    // RGB color, W intensity
    float4 color_intensity;

    // X outer cone cosine, Y inner cone cosine
    float4 outer_inner_cos;
};

LightSample sample_spot_light(SpotLight light, float3 position)
{
    LightSample sample;
    sample.direction = float3(0.0f, 0.0f, 1.0f);
    sample.radiance = 0.0f;

    float3 to_light = light.position_range.xyz - position;
    float distance_to_light = length(to_light);

    if (distance_to_light >= light.position_range.w ||
        distance_to_light <= 0.0001f)
    {
        return sample;
    }

    sample.direction = to_light / distance_to_light;

    float3 light_to_surface = -sample.direction;

    float theta = dot(normalize(light.direction.xyz), light_to_surface);

    float outer_cos = light.outer_inner_cos.x;
    float inner_cos = light.outer_inner_cos.y;
    float cone = saturate( (theta - outer_cos) / max(inner_cos - outer_cos, 0.0001f));

    float range_ratio = distance_to_light / light.position_range.w;
    float range_fade = saturate(1.0f - range_ratio * range_ratio);

    float attenuation = range_fade * range_fade / max(distance_to_light * distance_to_light, 0.0001f);

    sample.radiance = light.color_intensity.rgb * light.color_intensity.w * attenuation * cone;

    return sample;
}

#endif // OMEGA_SPOT_LIGHT_HLSLI