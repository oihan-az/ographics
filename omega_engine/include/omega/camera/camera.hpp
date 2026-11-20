#ifndef OMEGA_CAMERA_HPP
#define OMEGA_CAMERA_HPP

#include <omega/core/math/projection.hpp>

#include <omega/core/component/component_traits.hpp>
#include <omega/transform/transform.hpp> 

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

template<>
struct ComponentTraits<PerspectiveCamera>
{
    using Dependencies = TypeList<Transform>;
};

template<>
struct ComponentTraits<OrthogonalCamera>
{
    using Dependencies = TypeList<Transform>;
};

} // namespace omega

#endif // OMEGA_CAMERA_HPP