#ifndef OMEGA_GPU_LIGHT_DATA_HPP
#define OMEGA_GPU_LIGHT_DATA_HPP

#include <omega/core/math/vector4.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/transform/transform_system.hpp>

namespace omega
{

template <typename T>
struct GpuLightData {};

template <>
struct GpuLightData<DirectionalLight>
{
    Vector4 direction{0.0f};
    Vector4 color_intensity{0.0f};
};

template <>
struct GpuLightData<PointLight>
{
    Vector4 position_range{0.0f};
    Vector4 color_intensity{0.0f};
};

template <>
struct GpuLightData<SpotLight>
{
    Vector4 position_range{0.0f};
    Vector4 direction{0.0f};
    Vector4 color_intensity{0.0f};
    Vector4 outer_inner_cos{0.0f};
};

template <typename T> 
GpuLightData<T> gpu_light_data(Entity entity, const T& ligh, TransformSystem& transforms) {}

template <> 
inline GpuLightData<DirectionalLight> gpu_light_data(Entity entity, const DirectionalLight& light,
                                                     TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuLightData<DirectionalLight> result{};
    result.direction = Vector4(normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f))), 0.0f);
    result.color_intensity = Vector4(light.color, light.intensity);

    return result;
}

template <> 
inline GpuLightData<PointLight> gpu_light_data(Entity entity, const PointLight& light, TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuLightData<PointLight> result{};
    result.position_range = Vector4(Vector3(world[3]), light.range);
    result.color_intensity = Vector4(light.color, light.intensity);

    return result;
}

template <> 
inline GpuLightData<SpotLight> gpu_light_data(Entity entity, const SpotLight& light, TransformSystem& transforms)
{
    const Matrix4& world = transforms.get_world_matrix(entity);

    GpuLightData<SpotLight> result{};
    result.position_range = Vector4(Vector3(world[3]), light.range);
    result.direction = Vector4(normalize(Vector3(world * Vector4(0.0f, 0.0f, -1.0f, 0.0f))), 0.0f);
    result.color_intensity = Vector4(light.color, light.intensity);
    result.outer_inner_cos = Vector4(std::cos(light.outer_angle * 0.5f), std::cos(light.inner_angle * 0.5f), 0.0f, 0.0f);

    return result;
}

} // namespace omega

#endif // OMEGA_GPU_LIGHT_DATA_HPP