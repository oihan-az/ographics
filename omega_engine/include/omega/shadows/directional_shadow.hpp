#ifndef OMEGA_DIRECTIONAL_SHADOW_HPP
#define OMEGA_DIRECTIONAL_SHADOW_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/lighting/directional_light.hpp> 
#include <omega/shadows/shadow_map.hpp>
#include <omega/shadows/shadow_compare.hpp>
#include <omega/shadows/shadow_filter.hpp>

#include <cstdint>

namespace omega
{

struct DirectionalShadow
{
    ShadowMapSettings map{2048};
    ShadowCompare compare{};
    ShadowFilter filter{};
};

template<>
struct ComponentTraits<DirectionalShadow>
{
    using Dependencies = TypeList<DirectionalLight>;
};

} // namespace omega

#endif // OMEGA_DIRECTIONAL_SHADOW_HPP