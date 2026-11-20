#ifndef OMEGA_POINT_LIGHT_HPP
#define OMEGA_POINT_LIGHT_HPP

#include <omega/core/math/vector3.hpp>

#include <omega/core/component/component_traits.hpp>
#include <omega/transform/transform.hpp> 

namespace omega
{

struct PointLight
{
    Vector3 color{1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
};

template<>
struct ComponentTraits<PointLight>
{
    using Dependencies = TypeList<Transform>;
};

} // namespace omega

#endif // OMEGA_POINT_LIGHT_HPP