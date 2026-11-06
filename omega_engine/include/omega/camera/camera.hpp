#ifndef OMEGA_CAMERA_HPP
#define OMEGA_CAMERA_HPP

#include <omega/core/math/projection.hpp>

namespace omega
{

struct PerspectiveCamera
{
    PerspectiveProjection proj;
};

struct OrthogonalCamera
{
    OrthogonalProjection proj;
};

} // namespace omega

#endif // OMEGA_CAMERA_HPP