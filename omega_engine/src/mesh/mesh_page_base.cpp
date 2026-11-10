#include <omega/mesh/mesh_page_base.hpp>

#include <utility>
#include <algorithm>

namespace omega
{

MeshPageBase::MeshPageBase(uint32_t vertex_capacity, uint32_t index_capacity,
                           const std::vector<uint32_t>& binding_strides)
    : m_vertex_capacity(vertex_capacity), m_index_capacity(index_capacity),
      m_binding_strides(binding_strides), m_index_allocator(index_capacity)
{
    m_vertex_allocators.reserve(binding_strides.size());

    for (uint32_t stride : binding_strides)
    {
        m_vertex_allocators.emplace_back(vertex_capacity);
    }
}

bool MeshPageBase::allocate(uint32_t vertex_count, uint32_t index_count, MeshAllocation& allocation)
{
    if (m_next_allocation_id == 0)
    {
        return false;
    }

    if (vertex_count == 0)
        return false;

    if (vertex_count > m_vertex_capacity || index_count > m_index_capacity)
        return false;

    MeshAllocation result;
    result.vertex_count = vertex_count;
    result.index_count = index_count;

    result.vertex_offsets.reserve(m_vertex_allocators.size());

    // Allocate all vertex bindings
    for (auto& allocator : m_vertex_allocators)
    {
        uint32_t offset = 0;

        if (!allocator.allocate(vertex_count, offset))
        {
            // Roll back previously allocated bindings
            for (std::size_t i = 0; i < result.vertex_offsets.size(); ++i)
            {
                m_vertex_allocators[i].release(result.vertex_offsets[i], vertex_count);
            }

            return false;
        }

        result.vertex_offsets.push_back(offset);
    }

    // Allocate index range only when indexed geometry is used
    if (index_count > 0)
    {
        if (!m_index_allocator.allocate(index_count, result.index_offset))
        {
            // Roll back all vertex allocations.
            for (std::size_t i = 0; i < result.vertex_offsets.size(); ++i)
            {
                m_vertex_allocators[i].release(result.vertex_offsets[i], vertex_count);
            }

            return false;
        }
    }

    allocation = std::move(result);

    allocation.allocation_id = m_next_allocation_id;
    m_active_allocations.push_back(allocation);

    if (m_next_allocation_id == std::numeric_limits<uint64_t>::max())
    {
        m_next_allocation_id = 0;
    }
    else
    {
        ++m_next_allocation_id;
    }

    return true;
}

bool MeshPageBase::release(const MeshAllocation& allocation)
{
    if (!is_active(allocation))
        return false;

    // Release all vertex ranges
    for (std::size_t i = 0; i < m_vertex_allocators.size(); ++i)
    {
        if (!m_vertex_allocators[i].release(allocation.vertex_offsets[i], allocation.vertex_count))
            return false;
    }

    if (allocation.index_count > 0)
    {
        if (!m_index_allocator.release(allocation.index_offset, allocation.index_count))
            return false;
    }

    const auto it = std::find(m_active_allocations.begin(), m_active_allocations.end(), allocation);
    m_active_allocations.erase(it);
    return true;
}

uint32_t MeshPageBase::vertex_capacity() const
{
    return m_vertex_capacity;
}

uint32_t MeshPageBase::index_capacity() const
{
    return m_index_capacity;
}

uint32_t MeshPageBase::binding_count() const
{
    return static_cast<uint32_t>(m_binding_strides.size());
}

uint32_t MeshPageBase::binding_stride(uint32_t binding) const
{
    if (binding >= m_binding_strides.size())
        return 0;

    return m_binding_strides[binding];
}

bool MeshPageBase::is_active(const MeshAllocation& allocation) const
{
    const auto it = std::find(m_active_allocations.begin(), m_active_allocations.end(), allocation);
    if (it == m_active_allocations.end())
    {
        return false;
    }

    return true;
}

} // namespace omega