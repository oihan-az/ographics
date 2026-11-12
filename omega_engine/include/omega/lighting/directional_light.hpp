#ifndef OMEGA_DIRECTIONAL_LIGHT_HPP
#define OMEGA_DIRECTIONAL_LIGHT_HPP

#include <omega/core/math/vector3.hpp>

namespace omega
{

struct DirectionalLight
{
    Vector3 color{1.0f};
    float intensity = 1.0f;
};

} // namespace omega

#endif // OMEGA_DIRECTIONAL_LIGHT_HPP