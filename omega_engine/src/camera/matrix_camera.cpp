#include <omega/camera/camera.hpp>

#include <omega/core/math/matrix_projection.hpp>

namespace omega
{

Matrix4 mtx_perspective_projection(const PerspectiveCamera& camera)
{
    return perspective_projection(camera.proj);
}

Matrix4 mtx_orthogonal_projection(const OrthogonalCamera& camera)
{
    return orthogonal_projection(camera.proj);
}

} // namespace omega