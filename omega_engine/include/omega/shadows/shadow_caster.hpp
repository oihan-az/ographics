#ifndef OMEGA_SHADOW_CASTER_HPP
#define OMEGA_SHADOW_CASTER_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/renderable/pbr_opaque_renderable.hpp> 

namespace omega
{

struct ShadowCaster {};

template<>
struct ComponentTraits<ShadowCaster>
{
    using Dependencies = TypeList<PbrOpaqueRenderable>;
};

} // namespace omega

#endif // OMEGA_SHADOW_CASTER_HPP