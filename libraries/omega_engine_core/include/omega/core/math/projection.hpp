#ifndef OMEGA_MATH_PROJECTION_HPP
#define OMEGA_MATH_PROJECTION_HPP

#include <omega/core/math/angle.hpp>

namespace omega
{

struct PerspectiveProjection
{
    float field_of_view = radians(60.0f);
    float aspect_ratio = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
};

struct OrthogonalProjection
{
    float left = -10.0f;
    float right = 10.0f;
    float bottom = -10.0f;
    float top = 10.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
};

} // namespace omega

#endif // OMEGA_MATH_PROJECTION_HPP