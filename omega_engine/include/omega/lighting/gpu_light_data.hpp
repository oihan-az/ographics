#ifndef OMEGA_GPU_LIGHT_DATA_HPP
#define OMEGA_GPU_LIGHT_DATA_HPP

#include <omega/graphics/gpu_component/gpu_component_data.hpp>

#include <omega/core/math/vector4.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/transform/transform_system.hpp>

namespace omega
{

template <>
struct GpuComponentData<DirectionalLight>
{
    Vector4 direction{0.0f};
    Vector4 color_intensity{0.0f};
};

template <>
struct GpuComponentData<PointLight>
{
    Vector4 position_range{0.0f};
    Vector4 color_intensity{0.0f};
};

template <>
struct GpuComponentData<SpotLight>
{
    Vector4 position_range{0.0f};
    Vector4 direction{0.0f};
    Vector4 color_intensity{0.0f};
    Vector4 outer_inner_cos{0.0f};
};


inline GpuComponentData<DirectionalLight> gpu_component_data(const DirectionalLight& light, Entity entity, TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuComponentData<DirectionalLight> result{};
    result.direction = Vector4(normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f))), 0.0f);
    result.color_intensity = Vector4(light.color, light.intensity);

    return result;
}

inline GpuComponentData<PointLight> gpu_component_data(const PointLight& light, Entity entity, TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuComponentData<PointLight> result{};
    result.position_range = Vector4(Vector3(world[3]), light.range);
    result.color_intensity = Vector4(light.color, light.intensity);

    return result;
}

inline GpuComponentData<SpotLight> gpu_component_data(const SpotLight& light, Entity entity, TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuComponentData<SpotLight> result{};
    result.position_range = Vector4(Vector3(world[3]), light.range);
    result.direction = Vector4(normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f))), 0.0f);
    result.color_intensity = Vector4(light.color, light.intensity);
    result.outer_inner_cos = Vector4(std::cos(light.outer_angle * 0.5f), std::cos(light.inner_angle * 0.5f), 0.0f, 0.0f);

    return result;
}

} // namespace omega

#endif // OMEGA_GPU_LIGHT_DATA_HPP