#ifndef OMEGA_POINT_LIGHT_HLSLI
#define OMEGA_POINT_LIGHT_HLSLI

#include "light_sample.hlsli"

struct PointLight
{
    // XYZ position, W range
    float4 position_range;

    // RGB color, W intensity
    float4 color_intensity;
};

LightSample sample_point_light(PointLight light, float3 position)
{
    LightSample sample;
    sample.direction = float3(0.0f, 0.0f, 1.0f);
    sample.radiance = 0.0f;

    float3 to_light = light.position_range.xyz - position;
    float distance_to_light = length(to_light);
    if (distance_to_light >= light.position_range.w || distance_to_light <= 0.0001f)
    {
        return sample;
    }

    sample.direction = to_light / distance_to_light;

    // Inverse-square attenuation and smooth range cutoff
    float range_ratio = distance_to_light / light.position_range.w;
    float range_fade = saturate(1.0f - range_ratio * range_ratio);
    float attenuation = range_fade * range_fade / max(distance_to_light * distance_to_light, 0.0001f);

    sample.radiance = light.color_intensity.rgb * light.color_intensity.w * attenuation;

    return sample;
}

#endif // OMEGA_POINT_LIGHT_HLSLI