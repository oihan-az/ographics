#ifndef OMEGA_DIRECTIONAL_LIGHT_HLSLI
#define OMEGA_DIRECTIONAL_LIGHT_HLSLI

#include "light_sample.hlsli"

struct DirectionalLight
{
    // XYZ direction
    float4 direction;

    // RGB color, W intensity.
    float4 color_intensity;
};

LightSample sample_directional_light(DirectionalLight light)
{
    LightSample sample;

    sample.direction = normalize(-light.direction.xyz);
    sample.radiance = light.color_intensity.rgb * light.color_intensity.w;

    return sample;
}

#endif // OMEGA_DIRECTIONAL_LIGHT_HLSLI