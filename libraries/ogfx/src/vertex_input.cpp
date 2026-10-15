#include <ogfx/vertex_input.hpp>

namespace ogfx
{

const char* to_string(VertexInputRate rate)
{
    switch (rate)
    {
    case VertexInputRate::Vertex:
        return "Vertex";
    case VertexInputRate::Instance:
        return "Instance";
    }

    return "Unknown";
}

} // namespace ogfx