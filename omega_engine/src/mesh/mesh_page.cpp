#include <omega/mesh/mesh_page.hpp>

#include <utility>
#include <algorithm>

namespace omega
{

MeshPage::MeshPage(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides)
    : MeshPageBase(vertex_capacity, index_capacity, binding_strides)
{
    m_vertex_data.reserve(binding_strides.size());

    for (uint32_t stride : binding_strides)
    {
        m_vertex_data.emplace_back(static_cast<std::size_t>(vertex_capacity) * stride);
    }

    m_indices.resize(index_capacity);
}

bool MeshPage::write(const MeshAllocation& allocation, 
                     const std::vector<std::span<const std::byte>>& vertex_data,
                     const std::span<const uint32_t> indices)
{
    if (!is_active(allocation))
        return false;

    uint32_t binding_count = this->binding_count();

    if (vertex_data.size() != binding_count ||
        allocation.vertex_offsets.size() != binding_count || 
        indices.size() != allocation.index_count)
    {
        return false;
    }

    for (size_t i = 0; i < vertex_data.size(); ++i)
    {
        const size_t expected_size = static_cast<size_t>(allocation.vertex_count) * binding_stride(i);
        if (vertex_data[i].size() != expected_size)
        {
            return false;
        }
    }

    for (size_t i = 0; i < vertex_data.size(); ++i)
    {
        const size_t destination_offset = static_cast<size_t>(allocation.vertex_offsets[i]) * binding_stride(i);
        std::copy(vertex_data[i].begin(), vertex_data[i].end(), m_vertex_data[i].begin() + destination_offset);
    }

    if (!indices.empty())
    {
        std::copy(indices.begin(), indices.end(), m_indices.begin() + allocation.index_offset);
    }

    return true;
}

const std::vector<std::byte>& MeshPage::vertex_data(uint32_t binding) const
{
    static const std::vector<std::byte> empty;

    if (binding >= m_vertex_data.size())
        return empty;

    return m_vertex_data[binding];
}

const std::vector<uint32_t>& MeshPage::indices() const
{
    return m_indices;
}

} // namespace omega