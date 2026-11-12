#ifndef OMEGA_POINT_LIGHT_HPP
#define OMEGA_POINT_LIGHT_HPP

#include <omega/core/math/vector3.hpp>

namespace omega
{

struct PointLight
{
    Vector3 color{1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
};

} // namespace omega

#endif // OMEGA_POINT_LIGHT_HPP