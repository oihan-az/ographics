#ifndef OMEGA_MESH_DATA_HPP
#define OMEGA_MESH_DATA_HPP

#include <omega/mesh/mesh_handle.hpp>
#include <omega/mesh/vertex_layout.hpp>
#include <omega/mesh/mesh_page_allocation.hpp>

#include <cstdint>

namespace omega
{

struct MeshData
{
    MeshHandle handle;
    VertexLayoutId layout;
    MeshPageAllocation allocation;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    bool is_indexed() const { return index_count > 0; }
};

} // namespace omega

#endif // OMEGA_MESH_DATA_HPP