#ifndef OMEGA_MATH_MATRIX4_HPP
#define OMEGA_MATH_MATRIX4_HPP

#include <glm/mat4x4.hpp>

namespace omega
{

using Matrix4 = glm::mat4;

Matrix4 inverse(const Matrix4& matrix);
Matrix4 transpose(const Matrix4& matrix);
float determinant(const Matrix4& matrix);

} // namespace omega

#endif // OMEGA_MATH_MATRIX4_HPP