#include <omega/mesh/mesh_layout_group.hpp>

#include <algorithm>
#include <utility>

namespace omega
{

MeshLayoutGroup::MeshLayoutGroup(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                                 std::vector<uint32_t> binding_strides)
    : m_default_vertex_capacity(default_vertex_capacity), m_default_index_capacity(default_index_capacity),
      m_binding_strides(std::move(binding_strides))
{
}

bool MeshLayoutGroup::allocate(uint32_t vertex_count, uint32_t index_count, MeshPageAllocation& out_allocation)
{
    if (vertex_count == 0)
    {
        return false;
    }

    // Try existing pages first
    for (uint32_t i = 0; i < m_pages.size(); ++i)
    {
        MeshAllocation allocation;

        if (m_pages[i].allocate(vertex_count, index_count, allocation))
        {
            out_allocation.page_index = i;
            out_allocation.allocation = std::move(allocation);
            return true;
        }
    }

    // No existing page could satisfy the request
    // Create a page large enough for this allocation
    const uint32_t vertex_capacity = std::max(m_default_vertex_capacity, vertex_count);
    const uint32_t index_capacity = std::max(m_default_index_capacity, index_count);
    m_pages.emplace_back(vertex_capacity, index_capacity, m_binding_strides);

    const uint32_t page_index = static_cast<uint32_t>(m_pages.size() - 1);

    MeshAllocation allocation;
    if (!m_pages.back().allocate(vertex_count, index_count, allocation))
    {
        m_pages.pop_back();
        return false;
    }

    out_allocation.page_index = page_index;
    out_allocation.allocation = std::move(allocation);
    return true;
}

bool MeshLayoutGroup::release(const MeshPageAllocation& allocation)
{
    if (allocation.page_index >= m_pages.size())
    {
        return false;
    }

    return m_pages[allocation.page_index].release(allocation.allocation);
}

bool MeshLayoutGroup::write(const MeshPageAllocation& allocation,
                            const std::vector<std::vector<std::byte>>& vertex_data,
                            const std::vector<uint32_t>& indices)
{
    if (allocation.page_index >= m_pages.size())
    {
        return false;
    }

    return m_pages[allocation.page_index].write(allocation.allocation, vertex_data, indices);
}

uint32_t MeshLayoutGroup::page_count() const
{
    return static_cast<uint32_t>(m_pages.size());
}

} // namespace omega