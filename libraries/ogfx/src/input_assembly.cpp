#include <ogfx/input_assembly.hpp>

namespace ogfx
{

const char* to_string(PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::Undefined:
        return "Undefined";
    case PrimitiveTopology::PointList:
        return "PointList";
    case PrimitiveTopology::LineList:
        return "LineList";
    case PrimitiveTopology::LineStrip:
        return "LineStrip";
    case PrimitiveTopology::TriangleList:
        return "TriangleList";
    case PrimitiveTopology::TriangleStrip:
        return "TriangleStrip";
    case PrimitiveTopology::TriangleFan:
        return "TriangleFan";
    case PrimitiveTopology::LineListWithAdjacency:
        return "LineListWithAdjacency";
    case PrimitiveTopology::LineStripWithAdjacency:
        return "LineStripWithAdjacency";
    case PrimitiveTopology::TriangleListWithAdjacency:
        return "TriangleListWithAdjacency";
    case PrimitiveTopology::TriangleStripWithAdjacency:
        return "TriangleStripWithAdjacency";
    case PrimitiveTopology::PatchList:
        return "PatchList";
    }

    return "Unknown";
}

} // namespace ogfx