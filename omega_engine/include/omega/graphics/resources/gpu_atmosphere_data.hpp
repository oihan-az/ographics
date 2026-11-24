#ifndef OMEGA_GPU_ATMOSPHERE_DATA_HPP
#define OMEGA_GPU_ATMOSPHERE_DATA_HPP

#include <omega/graphics/resources/gpu_component_data.hpp>
#include <omega/graphics/resources/gpu_light_data.hpp>

#include <omega/atmosphere/atmosphere.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/transform/transform_system.hpp>

namespace omega
{

template <> 
struct GpuComponentData<Atmosphere>
{
    Vector4 camera_position{0.0f};
    Vector4 sun_direction{0.0f};
    Vector4 sun_radiance{0.0f};
    Vector4 planet{0.0f};
    Vector4 scattering{0.0f};
    Vector4 settings{0.0f};
};

inline GpuComponentData<Atmosphere> 
gpu_component_data(const Atmosphere& atmosphere, const DirectionalLight& sun,
                                                       Entity atmosphere_entity, Entity camera_entity,
                                                       TransformSystem& transforms)
{
    GpuComponentData<Atmosphere> result{};

    // Sun
    const GpuComponentData<DirectionalLight> sun_data = gpu_component_data(sun, atmosphere_entity, transforms);
    result.sun_direction = -sun_data.direction;
    result.sun_radiance = Vector4(Vector3(sun_data.color_intensity) * sun_data.color_intensity.w, 0.0f);

    // Camera
    const Matrix4& camera_world = transforms.get_world_matrix(camera_entity);
    const Vector3 camera_position = Vector3(camera_world[3]);
    result.camera_position = Vector4(camera_position.x, camera_position.y + atmosphere.planet_radius + atmosphere.ground_offset, camera_position.z, 0.0f);

    // Planet
    result.planet = Vector4(atmosphere.planet_radius, atmosphere.atmosphere_radius, atmosphere.rayleigh_scale_height, atmosphere.mie_scale_height);

    // Scattering
    result.scattering = Vector4(atmosphere.rayleigh_scattering, atmosphere.mie_scattering);
    result.settings = Vector4(atmosphere.mie_anisotropy, 0.0f, 0.0f, 0.0f);

    return result;
}

} // namespace omega

#endif // OMEGA_GPU_ATMOSPHERE_DATA_HPP
