#ifndef OMEGA_MESH_PAGE_BASE_HPP
#define OMEGA_MESH_PAGE_BASE_HPP

#include <omega/core/memory/range_allocator.hpp>
#include <omega/mesh/mesh_allocation.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>

namespace omega
{

class MeshPageBase
{
  public:
    MeshPageBase(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides);

    virtual ~MeshPageBase() = default;

    bool allocate(uint32_t vertex_count, uint32_t index_count, MeshAllocation& allocation);
    bool release(const MeshAllocation& allocation);

    uint32_t vertex_capacity() const;
    uint32_t index_capacity() const;

    uint32_t binding_count() const;
    uint32_t binding_stride(uint32_t binding) const;

protected:
    bool is_active(const MeshAllocation& allocation) const;

  private:
    uint32_t m_vertex_capacity = 0;
    uint32_t m_index_capacity = 0;
    std::vector<uint32_t> m_binding_strides;

    std::vector<RangeAllocator> m_vertex_allocators;
    RangeAllocator m_index_allocator;

    std::vector<MeshAllocation> m_active_allocations;
    uint64_t m_next_allocation_id = 1;
};

} // namespace omega

#endif // OMEGA_MESH_PAGE_HPP