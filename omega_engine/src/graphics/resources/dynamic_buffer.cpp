#include <omega/graphics/resources/dynamic_buffer.hpp>

#include <orendergraph/render_graph_core.hpp>
#include <orendergraph/resource_traits/buffer.hpp>

#include <limits>

namespace omega
{

bool DynamicBuffer::create(orendergraph::RenderGraph& graph, const ogfx::BufferDesc& desc)
{
    if (m_handle.is_valid() || desc.size == 0)
        return false;

    m_desc = desc;
    m_handle = graph.create_resource<ogfx::Buffer>(m_desc);

    return m_handle.is_valid();
}

bool DynamicBuffer::ensure_capacity(orendergraph::RenderGraph& graph, uint64_t required_size)
{
    if (!m_handle.is_valid() || required_size == 0)
        return false;

    const uint64_t current_capacity = m_desc.size;

    if (required_size <= current_capacity)
        return true;

    uint64_t new_capacity = current_capacity;

    while (new_capacity < required_size)
    {
        if (new_capacity > std::numeric_limits<uint64_t>::max() / 2)
        {
            new_capacity = required_size;
            break;
        }

        new_capacity *= 2;
    }

    ogfx::BufferDesc new_desc = m_desc;
    new_desc.size = new_capacity;

    if (!graph.replace_resource(m_handle, new_desc))
        return false;

    m_desc = new_desc;

    return true;
}

void DynamicBuffer::release(orendergraph::RenderGraph& graph)
{
    if (!m_handle.is_valid())
        return;

    graph.release_resource(m_handle);

    m_handle = {};
    m_desc = {};
}

orendergraph::ResourceHandle<ogfx::Buffer> DynamicBuffer::get_handle() const
{
    return m_handle;
}

uint64_t DynamicBuffer::get_capacity() const
{
    return m_desc.size;
}

} // namespace omega
