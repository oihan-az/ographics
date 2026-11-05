#ifndef OMEGA_MATH_MATRIX_TRANSFORM_HPP
#define OMEGA_MATH_MATRIX_TRANSFORM_HPP

#include <omega/math/matrix4.hpp>
#include <omega/math/quaternion.hpp>
#include <omega/math/vector3.hpp>

namespace omega
{

Matrix4 translate_mtx(const Vector3& translation);
Matrix4 rotate_mtx(const Quaternion& rotation);
Matrix4 scale_mtx(const Vector3& scale);

} // namespace omega

#endif // OMEGA_MATH_MATRIX_TRANSFORM_HPP