#ifndef OMEGA_MATERIAL_RENDERABLE_HPP
#define OMEGA_MATERIAL_RENDERABLE_HPP

#include <omega/material/material_handle.hpp>
#include <omega/mesh/mesh_handle.hpp>

namespace omega
{

template <typename T> 
struct MaterialRenderable
{
    MeshHandle mesh;
    MaterialHandle<T> material;
};

} // namespace omega

#endif // OMEGA_MATERIAL_RENDERABLE_HPP