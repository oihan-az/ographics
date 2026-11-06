#ifndef OMEGA_TRANSFORM_HPP
#define OMEGA_TRANSFORM_HPP

#include <omega/core/math/vector3.hpp>
#include <omega/core/math/quaternion.hpp>
#include <omega/core/math/matrix4.hpp>

namespace omega
{

struct Transform
{
    Vector3 position{0.0f};
    Quaternion rotation{1.0f, 0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f};
};

} // namespace omega

#endif // OMEGA_TRANSFORM_HPP