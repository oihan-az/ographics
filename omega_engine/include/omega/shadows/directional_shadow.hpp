#ifndef OMEGA_DIRECTIONAL_SHADOW_HPP
#define OMEGA_DIRECTIONAL_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/directional_light.hpp> 

#include <cstdint>

namespace omega
{

struct DirectionalShadow
{
    uint32_t resolution = 2048;
    float depth_bias = 0.001f;
    float normal_bias = 0.01f;
};

template<>
struct ComponentTraits<DirectionalShadow>
{
    using Dependencies = TypeList<DirectionalLight>;
};

} // namespace omega

#endif // OMEGA_DIRECTIONAL_SHADOW_HPP