#ifndef OMEGA_DIRECTIONAL_LIGHT_HPP
#define OMEGA_DIRECTIONAL_LIGHT_HPP

#include <omega/core/math/vector3.hpp>

#include <omega/core/component/component_traits.hpp>
#include <omega/transform/transform.hpp> 

namespace omega
{

struct DirectionalLight
{
    Vector3 color{1.0f};
    float intensity = 1.0f;
};

template<>
struct ComponentTraits<DirectionalLight>
{
    using Dependencies = TypeList<Transform>;
};

} // namespace omega

#endif // OMEGA_DIRECTIONAL_LIGHT_HPP