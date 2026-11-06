#ifndef OMEGA_MATRIX_TRANSFORM_HPP
#define OMEGA_MATRIX_TRANSFORM_HPP

#include <omega/core/math/matrix4.hpp>
#include <omega/transform/transform.hpp>

namespace omega
{

Matrix4 mtx_transform(const Transform& tr);

} // namespace omega

#endif // OMEGA_MATRIX_TRANSFORM_HPP