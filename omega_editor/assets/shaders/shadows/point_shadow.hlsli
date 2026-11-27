#ifndef OMEGA_POINT_SHADOW_HLSLI
#define OMEGA_POINT_SHADOW_HLSLI

#include "shadow_filter.hlsli"

struct PointShadow
{
    float4x4 view_projections[6];
    float4 parameters;
    ShadowFilter filter;
};

float point_shadow_depth(float3 light_to_fragment, float near_plane, float far_plane)
{
    const float major_axis_distance = max(max(abs(light_to_fragment.x), abs(light_to_fragment.y)), abs(light_to_fragment.z));
    return far_plane / (far_plane - near_plane) - (far_plane * near_plane) / ((far_plane - near_plane) * max(major_axis_distance, 1e-6f));
}

float point_shadow_linear_major_depth(
    float depth,
    float near_plane,
    float far_plane)
{
    return near_plane * far_plane / max(far_plane - depth * (far_plane - near_plane), 1e-6f);
}

void point_shadow_basis(
    float3 direction,
    out float3 tangent,
    out float3 bitangent)
{
    const float3 up = abs(direction.y) < 0.999f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
    tangent = normalize(cross(up, direction));
    bitangent = cross(direction, tangent);
}

float3 point_shadow_offset_direction(
    float3 direction,
    float3 tangent,
    float3 bitangent,
    float2 disk_offset,
    float angular_radius)
{
    return normalize(direction + (tangent * disk_offset.x + bitangent * disk_offset.y) * angular_radius);
}

float sample_point_shadow_pcf(
    TextureCube<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    float3 light_to_fragment,
    float near_plane,
    float far_plane,
    float depth_bias,
    float resolution,
    float radius_texels,
    uint sample_count,
    float2 rotation)
{
    const float receiver_distance = length(light_to_fragment);
    const float3 direction = light_to_fragment / receiver_distance;

    float3 tangent, bitangent;
    point_shadow_basis(direction, tangent, bitangent);

    // A cubemap face spans approximately two units in
    // tangent-plane coordinates
    const float angular_radius = max(radius_texels, 0.0f) * 2.0f / max(resolution, 1.0f);

    const uint count = shadow_sample_count(sample_count);
    float visibility = 0.0f;

    for (uint i = 0; i < count; ++i)
    {
        const float2 offset = shadow_disk_offset(i, count, rotation);

        const float3 sample_direction = point_shadow_offset_direction(
            direction,
            tangent,
            bitangent,
            offset,
            angular_radius
        );

        // Recompute projected receiver depth for each cubemap
        // direction: its major axis can change across a face seam
        const float reference_depth = point_shadow_depth(
            sample_direction * receiver_distance,
            near_plane,
            far_plane
        ) - depth_bias;

        visibility += shadow_map.SampleCmpLevelZero(
            shadow_sampler,
            sample_direction,
            reference_depth
        );
    }

    return visibility / float(count);
}

float sample_point_shadow_pcss(
    TextureCube<float> shadow_map,
    SamplerState depth_sampler,
    SamplerComparisonState shadow_sampler,
    float3 light_to_fragment,
    float near_plane,
    float far_plane,
    float depth_bias,
    float resolution,
    ShadowFilter filter,
    float2 rotation)
{
    const float receiver_distance = length(light_to_fragment);
    const float3 direction = light_to_fragment / receiver_distance;

    const float light_radius = max(filter.parameters.y, 0.0f);
    const float minimum_radius = max(filter.parameters.x, 0.0f);
    const float maximum_radius = max(filter.parameters.z, minimum_radius);

    if (light_radius <= 0.0f)
    {
        return sample_point_shadow_pcf(
            shadow_map,
            shadow_sampler,
            light_to_fragment,
            near_plane,
            far_plane,
            depth_bias,
            resolution,
            minimum_radius,
            filter.options.z,
            rotation
        );
    }

    float3 tangent, bitangent;
    point_shadow_basis(direction, tangent, bitangent);

    // Approximate angular blocker search region
    const float search_angle = min(light_radius / max(receiver_distance, 1e-5f), maximum_radius * 2.0f / max(resolution, 1.0f));

    // 1. Search the surrounding cubemap directions
    const uint blocker_count = shadow_sample_count(filter.options.y);
    float blocker_distance_sum = 0.0f;
    uint blockers = 0;
    for (uint i = 0; i < blocker_count; ++i)
    {
        const float3 sample_direction = point_shadow_offset_direction(direction, tangent, bitangent, shadow_disk_offset(i, blocker_count, rotation), search_angle);

        const float stored_depth = shadow_map.SampleLevel(depth_sampler, sample_direction, 0);
        const float reference_depth = point_shadow_depth(sample_direction * receiver_distance, near_plane, far_plane) - depth_bias;
        if (stored_depth >= reference_depth)
            continue;

        const float blocker_major_distance = point_shadow_linear_major_depth(stored_depth, near_plane, far_plane);
        const float major_component = max(max(abs(sample_direction.x), abs(sample_direction.y)), abs(sample_direction.z));

        // Convert cubemap major-axis distance back to
        // radial distance from the light
        const float blocker_distance = blocker_major_distance / max(major_component, 1e-5f);
        blocker_distance_sum += blocker_distance;
        ++blockers;
    }

    if (blockers == 0)
        return 1.0f;

    const float average_blocker_distance = blocker_distance_sum / float(blockers);

    // 2. Approximate angular penumbra size
    const float separation = max(receiver_distance - average_blocker_distance, 0.0f);
    const float penumbra_world = light_radius * separation / max(average_blocker_distance, 1e-5f);
    const float penumbra_angle = penumbra_world / max(receiver_distance, 1e-5f);
    const float radius_texels = clamp(penumbra_angle * resolution * 0.5f, minimum_radius, maximum_radius);

    // 3. Variable-radius cubemap PCF.
    return sample_point_shadow_pcf(
        shadow_map,
        shadow_sampler,
        light_to_fragment,
        near_plane,
        far_plane,
        depth_bias,
        resolution,
        radius_texels,
        filter.options.z,
        rotation
    );
}

float sample_point_shadow(
    TextureCube<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    SamplerState depth_sampler,
    PointShadow shadow,
    float3 light_position,
    float light_range,
    float3 world_position,
    float3 normal,
    float2 screen_position)
{
    const float3 sample_position = world_position + normal * shadow.parameters.y;
    const float3 light_to_fragment = sample_position - light_position;

    const float near_plane = shadow.parameters.z;
    const float far_plane = light_range;

    const float distance_to_light = length(light_to_fragment);
    if (distance_to_light <= near_plane || distance_to_light >= far_plane)
    {
        return 1.0f;
    }

    const float major_axis_distance = max(max(abs(light_to_fragment.x), abs(light_to_fragment.y)), abs(light_to_fragment.z));
    if (major_axis_distance <= near_plane)
        return 1.0f;

    const float depth_bias = shadow.parameters.x;
    const float resolution = shadow.parameters.w;
    const float reference_depth = point_shadow_depth(light_to_fragment, near_plane, far_plane);

    if (shadow.filter.options.x == SHADOW_FILTER_HARD)
    {
        return shadow_map.SampleCmpLevelZero(
            shadow_sampler,
            light_to_fragment,
            reference_depth - depth_bias
        );
    }
    
    const float2 rotation = shadow_rotation(screen_position);
    
    if (shadow.filter.options.x == SHADOW_FILTER_PCF)
    {
        return sample_point_shadow_pcf(
            shadow_map,
            shadow_sampler,
            light_to_fragment,
            near_plane,
            far_plane,
            depth_bias,
            resolution,
            shadow.filter.parameters.x,
            shadow.filter.options.z,
            rotation
        );
    }

    if (shadow.filter.options.x == SHADOW_FILTER_PCSS)
    {
        return sample_point_shadow_pcss(
            shadow_map,
            depth_sampler,
            shadow_sampler,
            light_to_fragment,
            near_plane,
            far_plane,
            depth_bias,
            resolution,
            shadow.filter,
            rotation
        );
    }

    // Original hard-shadow path
    return shadow_map.SampleCmpLevelZero(
        shadow_sampler,
        light_to_fragment,
        reference_depth - depth_bias
    );
}

#endif // OMEGA_POINT_SHADOW_HLSLI