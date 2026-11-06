#ifndef OMEGA_MATH_CONSTANTS_HPP
#define OMEGA_MATH_CONSTANTS_HPP

#include <glm/gtc/constants.hpp>

namespace omega
{

template <typename T> constexpr T pi() { return glm::pi<T>(); }
template <typename T> constexpr T half_pi() { return glm::half_pi<T>(); }

} // namespace omega

#endif // OMEGA_MATH_CONSTANTS_HPP