#ifndef OMEGA_SPOT_LIGHT_HLSLI
#define OMEGA_SPOT_LIGHT_HLSLI

struct SpotLight
{
    // XYZ position, W range
    float4 position_range;

    // XYZ direction
    float4 direction;

    // RGB color, W intensity
    float4 color_intensity;

    // X outer cone cosine, Y inner cone cosine, 
    float4 outer_inner_cos;
};

float3 evaluate_spot_light(SpotLight light, float3 position, float3 normal)
{
    float3 to_light = light.position_range.xyz - position;

    float distance_to_light = length(to_light);

    if (distance_to_light >= light.position_range.w || distance_to_light <= 0.0001)
        return 0.0;

    float3 light_direction = to_light / distance_to_light;

    // Direction from the light toward the surface
    float3 light_to_surface = -light_direction;

    float theta = dot(normalize(light.direction.xyz), light_to_surface);

    float outer_cos = light.outer_inner_cos.x;
    float inner_cos = light.outer_inner_cos.y;

    float cone = saturate((theta - outer_cos) / max(inner_cos - outer_cos, 0.0001));

    float diffuse = max(dot(normal, light_direction), 0.0);

    // Inverse-square attenuation with a smooth range cutoff
    float range_ratio = distance_to_light / light.position_range.w;
    float range_fade = saturate(1.0 - range_ratio * range_ratio);
    float attenuation = range_fade * range_fade / max(distance_to_light * distance_to_light, 0.0001);

    return light.color_intensity.rgb * light.color_intensity.w * diffuse * attenuation * cone;
}

#endif // OMEGA_SPOT_LIGHT_HLSLI