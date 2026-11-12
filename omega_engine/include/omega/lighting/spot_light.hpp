#ifndef OMEGA_SPOT_LIGHT_HPP
#define OMEGA_SPOT_LIGHT_HPP

#include <omega/core/math/vector3.hpp>
#include <omega/core/math/angle.hpp>

namespace omega
{

struct SpotLight
{
    Vector3 color{1.0f};
    float intensity = 1.0f;

    float range = 10.0f;
    float inner_angle = radians(20.0f);
    float outer_angle = radians(30.0f);
};

} // namespace omega

#endif // OMEGA_SPOT_LIGHT_HPP