#ifndef OMEGA_SHADOW_MAP_2D_HLSLI
#define OMEGA_SHADOW_MAP_2D_HLSLI

#include "shadow_filter.hlsli"

float shadow_linear_depth(
    float depth,
    float near_plane,
    float far_plane,
    bool perspective)
{
    if (!perspective)
        return near_plane + depth * (far_plane - near_plane);

    return near_plane * far_plane / max(far_plane - depth * (far_plane - near_plane), 1e-6f);
}

float sample_shadow_2d_pcf(
    Texture2D<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    float2 uv,
    float reference_depth,
    float radius_texels,
    float resolution,
    uint sample_count,
    float2 rotation)
{
    const uint count = shadow_sample_count(sample_count);
    const float texel_size = rcp(max(resolution, 1.0f));

    float visibility = 0.0f;

    for (uint i = 0; i < count; ++i)
    {
        const float2 offset = shadow_disk_offset(i, count, rotation) * radius_texels * texel_size;
        const float2 sample_uv = uv + offset;

        // Outside the shadow frustum: consider illuminated
        if (any(sample_uv < 0.0f) || any(sample_uv > 1.0f))
        {
            visibility += 1.0f;
            continue;
        }

        visibility += shadow_map.SampleCmpLevelZero(shadow_sampler, sample_uv, reference_depth);
    }

    return visibility / float(count);
}

float sample_shadow_2d_pcss(
    Texture2D<float> shadow_map,
    SamplerState depth_sampler,
    SamplerComparisonState shadow_sampler,
    float2 uv,
    float reference_depth,
    float near_plane,
    float far_plane,
    bool perspective,
    ShadowFilter filter,
    float resolution,
    float2 rotation)
{
    const float receiver_distance = shadow_linear_depth(reference_depth, near_plane, far_plane, perspective);

    const float light_size = max(filter.parameters.y, 0.0f);
    const float minimum_radius = max(filter.parameters.x, 0.0f);
    const float maximum_radius = max(filter.parameters.z, minimum_radius);

    if (light_size <= 0.0f)
    {
        return sample_shadow_2d_pcf(shadow_map, shadow_sampler, uv, reference_depth, minimum_radius, resolution, filter.options.z, rotation);
    }

    // The projection scale maps light-space distances into UV
    const float projection_scale = filter.parameters.w;

    // Directional: light_size is angular radius in radians
    // Spot: light_size is emitter radius in world units
    float search_radius_uv;
    if (perspective)
    {
        search_radius_uv = light_size * projection_scale / max(receiver_distance, 1e-5f);
    }
    else
    {
        // Approximate search region from the angular emitter size
        search_radius_uv = tan(light_size) * receiver_distance * projection_scale;
    }

    search_radius_uv = min(search_radius_uv, maximum_radius / max(resolution, 1.0f));

    // 1. Blocker search
    const uint blocker_count = shadow_sample_count(filter.options.y);
    float blocker_distance_sum = 0.0f;
    uint blockers = 0;
    for (uint i = 0; i < blocker_count; ++i)
    {
        const float2 sample_uv = uv + shadow_disk_offset(i, blocker_count, rotation) * search_radius_uv;
        if (any(sample_uv < 0.0f) || any(sample_uv > 1.0f))
            continue;

        const float stored_depth = shadow_map.SampleLevel(depth_sampler, sample_uv, 0);
        if (stored_depth < reference_depth)
        {
            blocker_distance_sum += shadow_linear_depth(stored_depth, near_plane, far_plane, perspective);
            ++blockers;
        }
    }

    if (blockers == 0)
        return 1.0f;

    const float average_blocker_distance = blocker_distance_sum / float(blockers);

    // 2. Estimate the penumbra
    const float separation = max(receiver_distance - average_blocker_distance, 0.0f);
    float penumbra_radius_uv;
    if (perspective)
    {
        // Perspective emitter geometry
        const float penumbra_world = light_size * separation / max(average_blocker_distance, 1e-5f);
        penumbra_radius_uv = penumbra_world * projection_scale /max(receiver_distance, 1e-5f);
    }
    else
    {
        // Orthographic directional shadow with angular light radius
        penumbra_radius_uv = separation * tan(light_size) * projection_scale;
    }

    const float radius_texels = clamp(penumbra_radius_uv * resolution, minimum_radius, maximum_radius);

    // 3. Variable-radius PCF
    return sample_shadow_2d_pcf(
        shadow_map,
        shadow_sampler,
        uv,
        reference_depth,
        radius_texels,
        resolution,
        filter.options.z,
        rotation
    );
}

float sample_shadow_2d(
    Texture2D<float> shadow_map,
    SamplerComparisonState shadow_sampler,
    SamplerState depth_sampler,
    float4x4 view_projection,
    float3 world_position,
    float3 normal,
    float depth_bias,
    float normal_bias,
    float near_plane,
    float far_plane,
    bool perspective,
    float resolution,
    ShadowFilter filter,
    float2 screen_position)
{
    const float3 sample_position = world_position + normal * normal_bias;
    const float4 clip = mul(view_projection, float4(sample_position, 1.0f));
    if (clip.w <= 0.0f)
        return 1.0f;

    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = ndc.xy * 0.5f + 0.5f;
    if (any(uv < 0.0f) || any(uv > 1.0f) || ndc.z < 0.0f || ndc.z > 1.0f)
    {
        return 1.0f;
    }

    const float reference_depth = ndc.z - depth_bias;

    if (filter.options.x == SHADOW_FILTER_HARD)
    {
        return shadow_map.SampleCmpLevelZero(
            shadow_sampler,
            uv,
            reference_depth
        );
    }
    
    const float2 rotation = shadow_rotation(screen_position);
    
    if (filter.options.x == SHADOW_FILTER_PCF)
    {
        return sample_shadow_2d_pcf(
            shadow_map,
            shadow_sampler,
            uv,
            reference_depth,
            filter.parameters.x,
            resolution,
            filter.options.z,
            rotation
        );
    }

    if (filter.options.x == SHADOW_FILTER_PCSS)
    {
        return sample_shadow_2d_pcss(
            shadow_map,
            depth_sampler,
            shadow_sampler,
            uv,
            reference_depth,
            near_plane,
            far_plane,
            perspective,
            filter,
            resolution,
            rotation
        );
    }

    // Original hard-shadow path.
    return shadow_map.SampleCmpLevelZero(
        shadow_sampler,
        uv,
        reference_depth
    );
}

#endif // OMEGA_SHADOW_MAP_2D_HLSLI