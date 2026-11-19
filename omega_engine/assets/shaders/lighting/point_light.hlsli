#ifndef OMEGA_POINT_LIGHT_HLSLI
#define OMEGA_POINT_LIGHT_HLSLI

struct PointLight
{
    // XYZ position, W range
    float4 position_range;

    // RGB color, W intensity
    float4 color_intensity;
};

float3 evaluate_point_light(PointLight light, float3 position, float3 normal)
{
    float3 to_light = light.position_range.xyz - position;

    float distance_to_light = length(to_light);

    if (distance_to_light >= light.position_range.w || distance_to_light <= 0.0001)
        return 0.0;

    float3 light_direction = to_light / distance_to_light;

    float diffuse = max(dot(normal, light_direction), 0.0);

    // Inverse-square attenuation with a smooth range cutoff
    float range_ratio = distance_to_light / light.position_range.w;
    float range_fade = saturate(1.0 - range_ratio * range_ratio);
    float attenuation = range_fade * range_fade / max(distance_to_light * distance_to_light, 0.0001);

    return light.color_intensity.rgb * light.color_intensity.w * diffuse * attenuation;
}

#endif // OMEGA_POINT_LIGHT_HLSLI