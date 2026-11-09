#ifndef OMEGA_VERTEX_LAYOUT_REGISTRY_HPP
#define OMEGA_VERTEX_LAYOUT_REGISTRY_HPP

#include <omega/mesh/vertex_layout.hpp>

#include <ogfx/vertex_input.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class VertexLayoutRegistry
{
public:
    VertexLayoutId register_layout(const VertexLayout& layout);
    bool contains(VertexLayoutId id) const;

    const VertexLayout* get(VertexLayoutId id) const;
    uint32_t size() const;

    void clear();

private:
    std::vector<VertexLayout> m_layouts;
};

} // namespace omega

#endif // OMEGA_VERTEX_LAYOUT_REGISTRY_HPP