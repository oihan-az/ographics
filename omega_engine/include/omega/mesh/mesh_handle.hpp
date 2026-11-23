#ifndef OMEGA_MESH_HANDLE_HPP
#define OMEGA_MESH_HANDLE_HPP

#include <omega/core/handle.hpp>

namespace omega
{

struct MeshTag;

using MeshHandle = Handle<MeshTag>;
using MeshHandleHash = HandleHash<MeshTag>;

} // namespace omega

#endif // OMEGA_MESH_HANDLE_HPP