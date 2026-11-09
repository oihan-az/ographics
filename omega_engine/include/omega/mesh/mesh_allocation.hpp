#ifndef OMEGA_MESH_ALLOCATION_HPP
#define OMEGA_MESH_ALLOCATION_HPP

#include <cstdint>
#include <vector>

namespace omega
{

struct MeshAllocation
{
    uint64_t allocation_id = 0;

    std::vector<uint32_t> vertex_offsets;
    uint32_t vertex_count = 0;

    uint32_t index_offset = 0;
    uint32_t index_count = 0;

    bool operator==(const MeshAllocation&) const = default;
};

} // namespace omega

#endif // OMEGA_MESH_ALLOCATION_HPP