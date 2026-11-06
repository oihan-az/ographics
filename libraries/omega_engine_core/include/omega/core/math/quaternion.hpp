#ifndef OMEGA_MATH_QUATERNION_HPP
#define OMEGA_MATH_QUATERNION_HPP

#include <omega/core/math/vector3.hpp>

#include <glm/gtc/quaternion.hpp>

namespace omega
{

using Quaternion = glm::quat;

Quaternion angle_axis(float angle, const Vector3& axis);

} // namespace omega

#endif // OMEGA_MATH_QUATERNION_HPP