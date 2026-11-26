#ifndef OMEGA_SPOT_SHADOW_HPP
#define OMEGA_SPOT_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/spot_light.hpp> 

#include <cstdint>

namespace omega
{

struct SpotShadow
{
    uint32_t resolution = 1024;
    float depth_bias = 0.001f;
    float normal_bias = 0.01f;
};

template<>
struct ComponentTraits<SpotShadow>
{
    using Dependencies = TypeList<SpotLight>;
};

} // namespace omega

#endif // OMEGA_SPOT_SHADOW_HPP