#ifndef OMEGA_MATRIX_CAMERA_HPP
#define OMEGA_MATRIX_CAMERA_HPP

#include <omega/core/math/matrix4.hpp>
#include <omega/camera/camera.hpp>

namespace omega
{

Matrix4 mtx_perspective_projection(const PerspectiveCamera& camera);
Matrix4 mtx_orthogonal_projection(const OrthogonalCamera& camera);

} // namespace omega

#endif // OMEGA_MATRIX_CAMERA_HPP