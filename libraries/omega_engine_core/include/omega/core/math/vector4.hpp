#ifndef OMEGA_MATH_VECTOR4_HPP
#define OMEGA_MATH_VECTOR4_HPP

#include <glm/vec4.hpp>

namespace omega
{

using Vector4 = glm::vec4;

Vector4 normalize(const Vector4& vec);
float dot(const Vector4& x, const Vector4& y);

} // namespace omega

#endif // OMEGA_MATH_VECTOR4_HPP