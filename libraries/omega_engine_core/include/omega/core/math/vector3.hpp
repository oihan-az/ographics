#ifndef OMEGA_MATH_VECTOR3_HPP
#define OMEGA_MATH_VECTOR3_HPP

#include <glm/vec3.hpp>

namespace omega
{

using Vector3 = glm::vec3;

Vector3 normalize(const Vector3& vec);
float dot(const Vector3& x, const Vector3& y);

} // namespace omega

#endif // OMEGA_MATH_VECTOR3_HPP