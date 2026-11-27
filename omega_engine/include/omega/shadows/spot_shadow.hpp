#ifndef OMEGA_SPOT_SHADOW_HPP
#define OMEGA_SPOT_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/spot_light.hpp> 
#include <omega/shadows/shadow_map.hpp>
#include <omega/shadows/shadow_compare.hpp>
#include <omega/shadows/shadow_filter.hpp>

#include <cstdint>

namespace omega
{

struct SpotShadow
{
    ShadowMapSettings map{1024};
    ShadowCompare compare{};
    ShadowFilter filter{};
};

template<>
struct ComponentTraits<SpotShadow>
{
    using Dependencies = TypeList<SpotLight>;
};

} // namespace omega

#endif // OMEGA_SPOT_SHADOW_HPP