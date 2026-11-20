#ifndef OMEGA_MATH_MATRIX4_HPP
#define OMEGA_MATH_MATRIX4_HPP

#include <omega/core/math/vector3.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/core/math/quaternion.hpp>

#include <glm/mat4x4.hpp>

namespace omega
{

using Matrix4 = glm::mat4;

Matrix4 inverse(const Matrix4& matrix);
Matrix4 transpose(const Matrix4& matrix);
float determinant(const Matrix4& matrix);
bool decompose(const Matrix4& matrix, Vector3& scale, Quaternion& rotation, Vector3& position, Vector3& skew, Vector4& perspective);

} // namespace omega

#endif // OMEGA_MATH_MATRIX4_HPP