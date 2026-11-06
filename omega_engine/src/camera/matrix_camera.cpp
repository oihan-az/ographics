#include <omega/camera/camera_projection.hpp>

#include <glm/ext/matrix_clip_space.hpp>

namespace omega
{

Matrix4 perspective_projection(const PerspectiveCamera& camera)
{
    return glm::perspective(camera.field_of_view, camera.aspect_ratio, camera.near_plane, camera.far_plane);
}

Matrix4 orthogonal_projection(const OrthogonalCamera& camera)
{
    return glm::ortho(camera.left, camera.right, camera.bottom, camera.top, camera.near_plane, camera.far_plane);
}

} // namespace omega