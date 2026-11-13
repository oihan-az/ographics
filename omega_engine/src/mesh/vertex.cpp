#include <omega/mesh/vertex.hpp>

#include <cstddef>

namespace omega
{

ogfx::VertexInputStateDesc standard_vertex_layout()
{
    ogfx::VertexInputStateDesc layout{};

    layout.bindings.push_back({.binding = 0, .stride = sizeof(Vertex), .input_rate = ogfx::VertexInputRate::Vertex});

    layout.attributes = 
    {
        {.location = 0, .binding = 0, .format = ogfx::Format::RGB32_FLOAT, .offset = offsetof(Vertex, position)},
        {.location = 1, .binding = 0, .format = ogfx::Format::RG32_FLOAT, .offset = offsetof(Vertex, uv)},
        {.location = 2, .binding = 0, .format = ogfx::Format::RGB32_FLOAT, .offset = offsetof(Vertex, normal)},
        {.location = 3, .binding = 0, .format = ogfx::Format::RGB32_FLOAT, .offset = offsetof(Vertex, tangent)}
    };

    return layout;
}

} // namespace omega
