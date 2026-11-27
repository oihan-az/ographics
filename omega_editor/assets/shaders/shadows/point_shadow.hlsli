#ifndef OMEGA_POINT_SHADOW_HLSLI
#define OMEGA_POINT_SHADOW_HLSLI

struct PointShadow
{
    float4x4 view_projections[6];
    float4 parameters;
};

float point_shadow_depth(float3 light_to_fragment, float near_plane, float far_plane)
{
    const float major_axis_distance = max(max(abs(light_to_fragment.x), abs(light_to_fragment.y)), abs(light_to_fragment.z));

    return far_plane / (far_plane - near_plane) - (far_plane * near_plane) / ((far_plane - near_plane) * max(major_axis_distance, 1e-6f));
}

float sample_point_shadow(
    TextureCube<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    PointShadow shadow,
    float3 light_position,
    float light_range,
    float3 world_position,
    float3 normal)
{
    const float3 sample_position = world_position + normal * shadow.parameters.y;

    const float3 light_to_fragment = sample_position - light_position;

    const float near_plane = shadow.parameters.z;
    const float far_plane = light_range;

    const float distance_to_light = length(light_to_fragment);
    if (distance_to_light <= near_plane || distance_to_light >= far_plane)
        return 1.0f;

    const float major_axis_distance = max(max(abs(light_to_fragment.x), abs(light_to_fragment.y)), abs(light_to_fragment.z));
    if (major_axis_distance <= near_plane)
        return 1.0f;

    const float reference_depth = point_shadow_depth(light_to_fragment, near_plane, far_plane);
    return shadow_map.SampleCmpLevelZero(shadow_sampler, light_to_fragment, reference_depth - shadow.parameters.x);
}

#endif // OMEGA_POINT_SHADOW_HLSLI