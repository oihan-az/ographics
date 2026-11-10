#ifndef OMEGA_MESH_SYSTEM_DESC_HPP
#define OMEGA_MESH_SYSTEM_DESC_HPP

#include <cstdint>

namespace omega
{
struct MeshSystemDesc
{
    uint32_t page_vertex_capacity = 65536;
    uint32_t page_index_capacity = 196608;
};
}

#endif // OMEGA_MESH_SYSTEM_DESC_HPP