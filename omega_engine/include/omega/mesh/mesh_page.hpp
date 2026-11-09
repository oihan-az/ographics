#ifndef OMEGA_MESH_PAGE_HPP
#define OMEGA_MESH_PAGE_HPP

#include <omega/core/memory/range_allocator.hpp>
#include <omega/mesh/mesh_allocation.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>

namespace omega
{

class MeshPage
{
  public:
    MeshPage(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides);

    bool allocate(uint32_t vertex_count, uint32_t index_count, MeshAllocation& allocation);
    bool release(const MeshAllocation& allocation);

    bool write(const MeshAllocation& allocation, 
               const std::vector<std::vector<std::byte>>& vertex_data,
               const std::vector<uint32_t>& indices);

    uint32_t vertex_capacity() const;
    uint32_t index_capacity() const;

  private:
    uint32_t m_vertex_capacity = 0;
    uint32_t m_index_capacity = 0;
    std::vector<uint32_t> m_binding_strides;

    std::vector<std::vector<std::byte>> m_vertex_data;
    std::vector<uint32_t> m_indices;

    std::vector<RangeAllocator> m_vertex_allocators;
    RangeAllocator m_index_allocator;

    std::vector<MeshAllocation> m_active_allocations;
    uint64_t m_next_allocation_id = 1;
};

} // namespace omega

#endif // OMEGA_MESH_PAGE_HPP