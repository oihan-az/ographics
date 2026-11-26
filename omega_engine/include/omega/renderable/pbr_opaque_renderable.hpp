#ifndef OMEGA_PBR_OPAQUE_RENDERABLE_HPP
#define OMEGA_PBR_OPAQUE_RENDERABLE_HPP

#include <omega/core/component/component_traits.hpp>
#include <omega/renderable/material_renderable.hpp>
#include <omega/material/materials/pbr_opaque_material.hpp>
#include <omega/transform/transform.hpp>

namespace omega
{

using PbrOpaqueRenderable = MaterialRenderable<PbrOpaqueMaterial>;

template<>
struct ComponentTraits<PbrOpaqueRenderable>
{
    using Dependencies = TypeList<Transform>;
};

} // namespace omega

#endif // OMEGA_PBR_OPAQUE_RENDERABLE_HPP