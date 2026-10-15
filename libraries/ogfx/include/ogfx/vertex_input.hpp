#ifndef OGFX_VERTEX_INPUT_HPP
#define OGFX_VERTEX_INPUT_HPP

#include <ogfx/format.hpp>

#include <cstdint>
#include <vector>

namespace ogfx
{

enum class VertexInputRate
{
    Vertex,
    Instance
};

const char* to_string(VertexInputRate rate);

struct VertexBindingDesc
{
    uint32_t binding = 0;
    uint32_t stride = 0;
    VertexInputRate input_rate = VertexInputRate::Vertex;
};

struct VertexAttributeDesc
{
    uint32_t location = 0;
    uint32_t binding = 0;
    Format format = Format::Undefined;
    uint32_t offset = 0;
};

struct VertexInputStateDesc
{
    std::vector<VertexBindingDesc> bindings;
    std::vector<VertexAttributeDesc> attributes;
};

} // namespace ogfx

#endif // OGFX_VERTEX_INPUT_HPP