#ifndef OMEGA_MATH_MATRIX_PROJECTION_HPP
#define OMEGA_MATH_MATRIX_PROJECTION_HPP

#include <omega/core/math/projection.hpp>
#include <omega/core/math/matrix4.hpp>

namespace omega
{

Matrix4 perspective_projection(const PerspectiveProjection& proj);
Matrix4 orthogonal_projection(const OrthogonalProjection& proj);

} // namespace omega

#endif // OMEGA_MATH_MATRIX_PROJECTION_HPP