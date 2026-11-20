#ifndef OMEGA_MATH_QUATERNION_HPP
#define OMEGA_MATH_QUATERNION_HPP

#include <omega/core/math/vector3.hpp>

#include <glm/gtc/quaternion.hpp>

namespace omega
{

using Quaternion = glm::quat;

Quaternion normalize(const Quaternion& quat);
Quaternion angle_axis(float angle, const Vector3& axis);
Vector3 euler_angles(const Quaternion& quat);

} // namespace omega

#endif // OMEGA_MATH_QUATERNION_HPP