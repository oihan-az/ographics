#ifndef OMEGA_GPU_SHADOW_DATA_HPP
#define OMEGA_GPU_SHADOW_DATA_HPP

#include <omega/graphics/resources/gpu_component_data.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/core/math/matrix4.hpp>
#include <omega/core/math/matrix_projection.hpp>

#include <omega/shadows/directional_shadow.hpp>
#include <omega/shadows/point_shadow.hpp>
#include <omega/shadows/spot_shadow.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/transform/transform_system.hpp>

#include <array>
#include <cstdint>

namespace omega
{

class TransformSystem;

struct ShadowProjectionSettings
{
    float directional_half_extent = 50.0f;
    float directional_near_plane = 0.05f;
    float directional_far_plane = 1000.0f;

    float spot_near_plane = 0.05f;
    float point_near_plane = 0.05f;
};

struct GpuShadowFilter
{
    // x = PCF radius (texels)
    // y = light size
    // z = maximum PCSS radius (texels)
    // w = light-space to UV projection scale for 2D PCSS
    Vector4 parameters{0.0f};

    // x = ShadowFilterMode
    // y = blocker sample count
    // z = filter sample count
    // w = unused
    std::array<uint32_t, 4> options{};
};

inline GpuShadowFilter gpu_shadow_filter(const ShadowFilter& filter)
{
    GpuShadowFilter result{};

    result.parameters = Vector4(std::max(filter.radius, 0.0f), std::max(filter.light_size, 0.0f),
                                std::max(filter.max_radius, 0.0f), 0.0f);

    result.options = {static_cast<uint32_t>(filter.mode), std::clamp(filter.blocker_samples, 1u, 64u),
                      std::clamp(filter.filter_samples, 1u, 64u), 0u};

    return result;
}

template<> 
struct GpuComponentData<DirectionalShadow>
{
    Matrix4 view_projection;
    Vector4 parameters{0.0f}; // depth bias, normal bias, unused, resolution
    GpuShadowFilter filter{};
    Vector4 projection_parameters{0.0f};
};

template<> 
struct GpuComponentData<SpotShadow>
{
    Matrix4 view_projection;
    Vector4 parameters{0.0f}; // depth bias, normal bias, near plane, resolution
    GpuShadowFilter filter{};
    Vector4 projection_parameters{0.0f};
};

template<> 
struct GpuComponentData<PointShadow>
{
    std::array<Matrix4, 6> view_projections;
    Vector4 parameters{0.0f}; // depth bias, normal bias, near plane, resolution
    GpuShadowFilter filter{};
};

inline GpuComponentData<DirectionalShadow> gpu_component_data(const DirectionalShadow& shadow, Entity entity,
                                                              TransformSystem& transforms, Entity camera,
                                                              const ShadowProjectionSettings& settings)
{
    const Matrix4& world = transforms.get_world_matrix(entity);
    const Matrix4& camera_world = transforms.get_world_matrix(camera);
    const Vector3 camera_position = Vector3(camera_world[3]);

    const Vector3 direction = normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f)));

    const Vector3 up_hint = normalize(Vector3(world[1]));
    const Vector3 right = normalize(cross(direction, up_hint));
    const Vector3 up = normalize(cross(right, direction));

    const float extent = settings.directional_half_extent;
    const float texel_size = (extent * 2.0f) / static_cast<float>(shadow.map.resolution);
    const float light_x = dot(camera_position, right);
    const float light_y = dot(camera_position, up);
    const float snapped_x = std::round(light_x / texel_size) * texel_size;
    const float snapped_y = std::round(light_y / texel_size) * texel_size;
    const Vector3 snapped_camera_position = camera_position + right * (snapped_x - light_x) + up * (snapped_y - light_y);

    // Position the shadow camera behind the scene region centered on the active camera
    Matrix4 shadow_world = world;
    const float depth_center = (settings.directional_near_plane + settings.directional_far_plane) * 0.5f;
    shadow_world[3] = Vector4(snapped_camera_position - direction * depth_center, 1.0f);
    const Matrix4 view = inverse(shadow_world);

    OrthogonalProjection projection{};
    projection.left = -extent;
    projection.right = extent;
    projection.bottom = -extent;
    projection.top = extent;
    projection.near_plane = settings.directional_near_plane;
    projection.far_plane = settings.directional_far_plane;

    Matrix4 projection_matrix = orthogonal_projection(projection);

    GpuComponentData<DirectionalShadow> result{};
    result.view_projection = projection_matrix * view;
    result.parameters = Vector4(shadow.compare.depth_bias, shadow.compare.normal_bias, settings.directional_near_plane, static_cast<float>(shadow.map.resolution));
    result.filter = gpu_shadow_filter(shadow.filter);
    result.filter.parameters.w = 1.0f / (2.0f * settings.directional_half_extent);
    result.projection_parameters = Vector4(settings.directional_near_plane, settings.directional_far_plane, 0.0f, 0.0f);

    return result;
}

// Spot shadow
inline GpuComponentData<SpotShadow> gpu_component_data(const SpotShadow& shadow, Entity entity,
                                                       ComponentSystem& components, TransformSystem& transforms,
                                                       const ShadowProjectionSettings& settings)
{
    const SpotLight* light = components.get<SpotLight>(entity);
    assert(light);

    const Matrix4& world = transforms.get_world_matrix(entity);
    const Matrix4 view = inverse(world);

    const float near_plane = std::max(settings.spot_near_plane, 0.001f);
    const float far_plane = std::max(light->range, near_plane + 0.01f);

    PerspectiveProjection projection{};
    projection.field_of_view = light->outer_angle;
    projection.aspect_ratio = 1.0f;
    projection.near_plane = near_plane;
    projection.far_plane = far_plane;

    GpuComponentData<SpotShadow> result{};
    result.view_projection = perspective_projection(projection) * view;
    result.parameters = Vector4(shadow.compare.depth_bias, shadow.compare.normal_bias, near_plane, static_cast<float>(shadow.map.resolution));
    result.filter = gpu_shadow_filter(shadow.filter);
    result.filter.parameters.w = 0.5f / std::tan(projection.field_of_view * 0.5f);
    result.projection_parameters = Vector4(near_plane, far_plane, 0.0f, 0.0f);

    return result;
}

// Point shadow
inline GpuComponentData<PointShadow> gpu_component_data(const PointShadow& shadow, Entity entity,
                                                        ComponentSystem& components, TransformSystem& transforms,
                                                        const ShadowProjectionSettings& settings)
{
    const PointLight* light = components.get<PointLight>(entity);
    assert(light);

    const Matrix4& world = transforms.get_world_matrix(entity);
    const Vector3 position = Vector3(world[3]);

    const float near_plane = std::max(settings.point_near_plane, 0.001f);
    const float far_plane = std::max(light->range, near_plane + 0.01f);

    PerspectiveProjection projection{};
    projection.field_of_view = radians(90.0f);
    projection.aspect_ratio = 1.0f;
    projection.near_plane = near_plane;
    projection.far_plane = far_plane;

    Matrix4 projection_matrix = perspective_projection(projection);
#if OMEGA_PROJECTION_FLIP_Y
    // Cubemap face views already use the required orientation
    projection_matrix[1][1] *= -1.0f;
#endif

    constexpr std::array<Vector3, 6> directions = 
    {
        {
            {1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, -1.0f}
        }
    };

    constexpr std::array<Vector3, 6> up_vectors = 
    {
        {
            {0.0f, -1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, -1.0f},
            {0.0f, -1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f}
        }
    };

    GpuComponentData<PointShadow> result{};
    for (uint32_t face = 0; face < 6; ++face)
    {
        const Matrix4 view = glm::lookAtRH(position, position + directions[face], up_vectors[face]);

        result.view_projections[face] = projection_matrix * view;
    }

    result.parameters = Vector4(shadow.compare.depth_bias, shadow.compare.normal_bias, near_plane, static_cast<float>(shadow.map.resolution));
    result.filter = gpu_shadow_filter(shadow.filter);

    return result;
}

} // namespace omega

#endif // OMEGA_GPU_SHADOW_DATA_HPP