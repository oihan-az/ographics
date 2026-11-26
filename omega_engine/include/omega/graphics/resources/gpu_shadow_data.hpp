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

#include <array>
#include <cstdint>

namespace omega
{

class TransformSystem;

struct ShadowProjectionSettings
{
    float directional_half_extent = 50.0f;
    float directional_depth = 200.0f;

    float spot_near_plane = 0.05f;
    float point_near_plane = 0.05f;
};

template<> 
struct GpuComponentData<DirectionalShadow>
{
    Matrix4 view_projection;
};

template<> 
struct GpuComponentData<SpotShadow>
{
    Matrix4 view_projection;
};

template<> 
struct GpuComponentData<PointShadow>
{
    std::array<Matrix4, 6> view_projections;
};

inline GpuComponentData<DirectionalShadow> gpu_component_data(const DirectionalShadow& shadow, Entity entity,
                                                              TransformSystem& transforms, Entity camera,
                                                              const ShadowProjectionSettings& settings)
{
    const Matrix4& world = transforms.get_world_matrix(entity);
    const Matrix4& camera_world = transforms.get_world_matrix(camera);
    const Vector3 camera_position = Vector3(camera_world[3]);

    const Vector3 direction = normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f)));

    // Position the shadow camera behind the scene region centered on the active camera
    Matrix4 shadow_world = world;
    shadow_world[3] = Vector4(camera_position - direction * settings.directional_depth, 1.0f);
    const Matrix4 view = inverse(shadow_world);

    const float extent = settings.directional_half_extent;
    OrthogonalProjection projection{};
    projection.left = -extent;
    projection.right = extent;
    projection.bottom = -extent;
    projection.top = extent;
    projection.near_plane = 0.1f;
    projection.far_plane = settings.directional_depth * 2.0f;

    GpuComponentData<DirectionalShadow> result{};
    result.view_projection = orthogonal_projection(projection) * view;

    return result;
}

// Spot shadow
inline GpuComponentData<SpotShadow> gpu_component_data(const SpotShadow& shadow, const SpotLight& light, Entity entity,
                                                       TransformSystem& transforms,
                                                       const ShadowProjectionSettings& settings)
{
    const Matrix4& world = transforms.get_world_matrix(entity);
    const Matrix4 view = inverse(world);

    const float near_plane = std::max(settings.spot_near_plane, 0.001f);
    const float far_plane = std::max(light.range, near_plane + 0.01f);

    PerspectiveProjection projection{};
    projection.field_of_view = light.outer_angle;
    projection.aspect_ratio = 1.0f;
    projection.near_plane = near_plane;
    projection.far_plane = far_plane;

    GpuComponentData<SpotShadow> result{};
    result.view_projection = perspective_projection(projection) * view;

    return result;
}

// Point shadow
inline GpuComponentData<PointShadow> gpu_component_data(const PointShadow& shadow, const PointLight& light,
                                                        Entity entity, TransformSystem& transforms,
                                                        const ShadowProjectionSettings& settings)
{
    const Matrix4& world = transforms.get_world_matrix(entity);
    const Vector3 position = Vector3(world[3]);

    const float near_plane = std::max(settings.point_near_plane, 0.001f);
    const float far_plane = std::max(light.range, near_plane + 0.01f);

    PerspectiveProjection projection{};
    projection.field_of_view = radians(90.0f);
    projection.aspect_ratio = 1.0f;
    projection.near_plane = near_plane;
    projection.far_plane = far_plane;

    const Matrix4 projection_matrix = perspective_projection(projection);

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

    return result;
}

} // namespace omega

#endif // OMEGA_GPU_SHADOW_DATA_HPP