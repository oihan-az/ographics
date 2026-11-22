#ifndef OMEGA_MATH_VECTOR2_HPP
#define OMEGA_MATH_VECTOR2_HPP

#include <glm/vec2.hpp>

namespace omega
{

using Vector2 = glm::vec2;

Vector2 normalize(const Vector2& vec);
float dot(const Vector2& x, const Vector2& y);

} // namespace omega

#endif // OMEGA_MATH_VECTOR2_HPP