#ifndef OMEGA_MESH_EVENT_HPP
#define OMEGA_MESH_EVENT_HPP

#include <omega/mesh/mesh_handle.hpp>

namespace omega
{

struct MeshCreatedEvent
{
    MeshHandle handle;
};

struct MeshReleasedEvent
{
    MeshHandle handle;
};

} // namespace omega

#endif // OMEGA_MESH_EVENT_HPP