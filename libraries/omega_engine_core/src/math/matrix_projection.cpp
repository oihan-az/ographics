#include <omega/core/math/matrix_projection.hpp>

#include <glm/ext/matrix_clip_space.hpp>

namespace omega
{

Matrix4 perspective_projection(const PerspectiveProjection& proj)
{
    Matrix4 projection;

#if OMEGA_PROJECTION_LEFT_HANDED
#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
    projection = glm::perspectiveLH_ZO(proj.field_of_view, proj.aspect_ratio, proj.near_plane, proj.far_plane);
#else
    projection = glm::perspectiveLH_NO(proj.field_of_view, proj.aspect_ratio, proj.near_plane, proj.far_plane);
#endif
#else
#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
    projection = glm::perspectiveRH_ZO(proj.field_of_view, proj.aspect_ratio, proj.near_plane, proj.far_plane);
#else
    projection = glm::perspectiveRH_NO(proj.field_of_view, proj.aspect_ratio, proj.near_plane, proj.far_plane);
#endif
#endif

#if OMEGA_PROJECTION_FLIP_Y
    projection[1][1] *= -1.0f;
#endif

    return projection;
}

Matrix4 orthogonal_projection(const OrthogonalProjection& proj)
{
    Matrix4 projection;

#if OMEGA_PROJECTION_LEFT_HANDED
#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
    projection = glm::orthoLH_ZO(proj.left, proj.right, proj.bottom, proj.top, proj.near_plane, proj.far_plane);
#else
    projection = glm::orthoLH_NO(proj.left, proj.right, proj.bottom, proj.top, proj.near_plane, proj.far_plane);
#endif
#else
#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
    projection = glm::orthoRH_ZO(proj.left, proj.right, proj.bottom, proj.top, proj.near_plane, proj.far_plane);
#else
    projection = glm::orthoRH_NO(proj.left, proj.right, proj.bottom, proj.top, proj.near_plane, proj.far_plane);
#endif
#endif

#if OMEGA_PROJECTION_FLIP_Y
    projection[1][1] *= -1.0f;
#endif

    return projection;
}

} // namespace omega