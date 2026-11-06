#ifndef OMEGA_MATRIX_CAMERA_HPP
#define OMEGA_MATRIX_CAMERA_HPP

#include <omega/core/math/matrix4.hpp>
#include <omega/camera/camera.hpp>

namespace omega
{

Matrix4 perspective_projection(const PerspectiveCamera& camera);
Matrix4 orthogonal_projection(const OrthogonalCamera& camera);

} // namespace omega

#endif // OMEGA_MATRIX_CAMERA_HPP