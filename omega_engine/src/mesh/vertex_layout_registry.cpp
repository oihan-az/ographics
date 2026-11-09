#include <omega/mesh/vertex_layout_registry.hpp>

namespace omega
{

VertexLayoutId VertexLayoutRegistry::register_layout(const VertexLayout& layout)
{
    for (uint32_t i = 0; i < m_layouts.size(); ++i)
    {
        if (m_layouts[i] == layout)
            return {i};
    }

    if (m_layouts.size() >= VertexLayoutId::invalid_index)
        return {};

    m_layouts.push_back(layout);

    return {static_cast<uint32_t>(m_layouts.size() - 1)};
}

bool VertexLayoutRegistry::contains(VertexLayoutId id) const
{
    return id.is_valid() && id.value < m_layouts.size();
}

const VertexLayout* VertexLayoutRegistry::get(VertexLayoutId id) const
{
    if (!contains(id))
        return nullptr;

    return &m_layouts[id.value];
}

uint32_t VertexLayoutRegistry::size() const
{
    return static_cast<uint32_t>(m_layouts.size());
}

void VertexLayoutRegistry::clear()
{
    m_layouts.clear();
}

} // namespace omega