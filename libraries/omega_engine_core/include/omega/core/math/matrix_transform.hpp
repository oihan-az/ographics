#ifndef OMEGA_MATH_MATRIX_TRANSFORM_HPP
#define OMEGA_MATH_MATRIX_TRANSFORM_HPP

#include <omega/core/math/matrix4.hpp>
#include <omega/core/math/quaternion.hpp>
#include <omega/core/math/vector3.hpp>

namespace omega
{

Matrix4 mtx_translate(const Vector3& translation);
Matrix4 mtx_rotate(const Quaternion& rotation);
Matrix4 mtx_scale(const Vector3& scale);

} // namespace omega

#endif // OMEGA_MATH_MATRIX_TRANSFORM_HPP