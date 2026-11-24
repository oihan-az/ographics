#ifndef OMEGA_ATMOSPHERE_SETTINGS_HPP
#define OMEGA_ATMOSPHERE_SETTINGS_HPP

#include <omega/core/math/vector3.hpp>

namespace omega
{

struct AtmosphereSettings
{
    // Planet geometry (meters)
    float planet_radius = 6360000.0f;
    float atmosphere_radius = 6460000.0f;

    // Atmospheric density scale heights (meters)
    float rayleigh_scale_height = 8000.0f;
    float mie_scale_height = 1200.0f;

    // Scattering coefficients (1/meters)
    Vector3 rayleigh_scattering{5.8e-6f, 13.5e-6f, 33.1e-6f};
    float mie_scattering = 21.0e-6f;

    // Mie anisotropy, typically between -1 and 1
    float mie_anisotropy = 0.76f;

    // Camera height offset above the surface (meters)
    float ground_offset = 2.0f;
};

} // namespace omega

#endif // OMEGA_ATMOSPHERE_SETTINGS_HPP
