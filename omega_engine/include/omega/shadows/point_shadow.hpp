#ifndef OMEGA_POINT_SHADOW_HPP
#define OMEGA_POINT_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/point_light.hpp> 
#include <omega/shadows/shadow_map.hpp>
#include <omega/shadows/shadow_compare.hpp>
#include <omega/shadows/shadow_filter.hpp>

#include <cstdint>

namespace omega
{

struct PointShadow
{
    ShadowMapSettings map{512};
    ShadowCompare compare{};
    ShadowFilter filter{};
};

template<>
struct ComponentTraits<PointShadow>
{
    using Dependencies = TypeList<PointLight>;
};

} // namespace omega

#endif // OMEGA_POINT_SHADOW_HPP