#ifndef OMEGA_RENDERABLE_HPP
#define OMEGA_RENDERABLE_HPP

#include <omega/mesh/mesh_handle.hpp>
#include <omega/core/math/vector3.hpp>

namespace omega
{

struct Renderable
{
    MeshHandle mesh;
    Vector3 color{1.0f, 1.0f, 1.0f};
};

} // namespace omega

#endif // OMEGA_RENDERABLE_HPP