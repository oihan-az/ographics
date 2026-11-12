#ifndef OMEGA_DIRECTIONAL_LIGHT_HLSLI
#define OMEGA_DIRECTIONAL_LIGHT_HLSLI

struct DirectionalLight
{
    // XYZ direction
    float4 direction;

    // RGB color, W intensity.
    float4 color_intensity;
};

float3 evaluate_directional_light(DirectionalLight light, float3 normal)
{
    float3 light_direction = normalize(-light.direction.xyz);

    float diffuse = max(dot(normal, light_direction), 0.0);
    
    return light.color_intensity.rgb * light.color_intensity.w * diffuse;
}

#endif // OMEGA_DIRECTIONAL_LIGHT_HLSLI