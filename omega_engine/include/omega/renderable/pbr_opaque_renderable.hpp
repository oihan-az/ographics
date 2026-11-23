#ifndef OMEGA_PBR_OPAQUE_RENDERABLE_HPP
#define OMEGA_PBR_OPAQUE_RENDERABLE_HPP

#include <omega/renderable/material_renderable.hpp>
#include <omega/material/materials/pbr_opaque_material.hpp>

namespace omega
{

using PbrOpaqueRenderable = MaterialRenderable<PbrOpaqueMaterial>;

} // namespace omega

#endif // OMEGA_PBR_OPAQUE_RENDERABLE_HPP