#ifndef OMEGA_MESH_PAGE_ALLOCATION_HPP
#define OMEGA_MESH_PAGE_ALLOCATION_HPP

#include <omega/mesh/mesh_allocation.hpp>

#include <cstdint>

namespace omega
{

struct MeshPageAllocation
{
    uint32_t page_index = 0;
    MeshAllocation allocation;

    bool operator==(const MeshPageAllocation&) const = default;
};

} // namespace omega

#endif // OMEGA_MESH_PAGE_ALLOCATION_HPP