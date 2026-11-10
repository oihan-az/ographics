#ifndef OMEGA_MESH_LAYOUT_GROUP_BASE_TPP
#define OMEGA_MESH_LAYOUT_GROUP_BASE_TPP

#include <algorithm>
#include <utility>

namespace omega
{

template <typename PageType>
MeshLayoutGroupBase<PageType>::MeshLayoutGroupBase(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                                                   std::vector<uint32_t> binding_strides)
    : m_default_vertex_capacity(default_vertex_capacity), m_default_index_capacity(default_index_capacity),
      m_binding_strides(std::move(binding_strides))
{
}

template <typename PageType>
bool MeshLayoutGroupBase<PageType>::allocate(uint32_t vertex_count, uint32_t index_count, MeshPageAllocation& out_allocation)
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
    PageType& page = create_page(vertex_capacity, index_capacity, m_binding_strides);

    const uint32_t page_index = static_cast<uint32_t>(m_pages.size() - 1);

    MeshAllocation allocation;
    if (!page.allocate(vertex_count, index_count, allocation))
    {
        m_pages.pop_back();
        return false;
    }

    out_allocation.page_index = page_index;
    out_allocation.allocation = std::move(allocation);
    return true;
}

template <typename PageType>
bool MeshLayoutGroupBase<PageType>::release(const MeshPageAllocation& allocation)
{
    if (allocation.page_index >= m_pages.size())
    {
        return false;
    }

    return m_pages[allocation.page_index].release(allocation.allocation);
}

template <typename PageType>
bool MeshLayoutGroupBase<PageType>::write(const MeshPageAllocation& allocation,
                                          const std::vector<std::span<const std::byte>>& vertex_data,
                                          const std::span<const uint32_t> indices)
{
    if (allocation.page_index >= m_pages.size())
    {
        return false;
    }

    return m_pages[allocation.page_index].write(allocation.allocation, vertex_data, indices);
}

template <typename PageType>
uint32_t MeshLayoutGroupBase<PageType>::page_count() const
{
    return static_cast<uint32_t>(m_pages.size());
}

template <typename PageType>
const PageType* MeshLayoutGroupBase<PageType>::page(uint32_t page_index) const
{
    if (page_index >= m_pages.size())
        return nullptr;

    return &m_pages[page_index];
}

} // namespace omega

#endif // OMEGA_MESH_LAYOUT_GROUP_BASE_TPP