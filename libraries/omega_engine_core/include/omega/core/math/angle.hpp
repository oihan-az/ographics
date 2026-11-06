#ifndef OMEGA_MATH_ANGLE_HPP
#define OMEGA_MATH_ANGLE_HPP

#include <glm/trigonometric.hpp>

namespace omega
{

template <typename T> constexpr T radians(T degrees) { return glm::radians(degrees); }
template <typename T> constexpr T degrees(T radians) { return glm::degrees(radians); }

} // namespace omega

#endif // OMEGA_MATH_ANGLE_HPP