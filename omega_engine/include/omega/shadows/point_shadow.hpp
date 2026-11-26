#ifndef OMEGA_POINT_SHADOW_HPP
#define OMEGA_POINT_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/point_light.hpp> 

#include <cstdint>

namespace omega
{

struct PointShadow
{
    uint32_t resolution = 512;
    float depth_bias = 0.001f;
    float normal_bias = 0.01f;
};

template<>
struct ComponentTraits<PointShadow>
{
    using Dependencies = TypeList<PointLight>;
};

} // namespace omega

#endif // OMEGA_POINT_SHADOW_HPP