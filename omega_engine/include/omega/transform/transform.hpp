#ifndef OMEGA_TRANSFORM_HPP
#define OMEGA_TRANSFORM_HPP

#include <omega/math/vector3.hpp>
#include <omega/math/quaternion.hpp>
#include <omega/math/matrix4.hpp>

namespace omega
{

struct Transform
{
    Vector3 position{0.0f};
    Quaternion rotation{1.0f, 0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f};

    Matrix4 matrix() const;
};

} // namespace omega

#endif // OMEGA_TRANSFORM_HPP