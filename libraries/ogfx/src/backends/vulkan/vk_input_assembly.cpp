#include "vk_input_assembly.hpp"

namespace ogfx
{

VkPrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::Undefined:
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;

    case PrimitiveTopology::PointList:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    case PrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    case PrimitiveTopology::LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

    case PrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    case PrimitiveTopology::TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    case PrimitiveTopology::TriangleFan:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

    case PrimitiveTopology::LineListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

    case PrimitiveTopology::LineStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;

    case PrimitiveTopology::TriangleListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;

    case PrimitiveTopology::TriangleStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;

    case PrimitiveTopology::PatchList:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }

    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

PrimitiveTopology from_vk_primitive_topology(VkPrimitiveTopology topology)
{
    switch (topology)
    {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return PrimitiveTopology::PointList;

    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return PrimitiveTopology::LineList;

    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return PrimitiveTopology::LineStrip;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return PrimitiveTopology::TriangleList;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return PrimitiveTopology::TriangleStrip;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        return PrimitiveTopology::TriangleFan;

    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        return PrimitiveTopology::LineListWithAdjacency;

    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        return PrimitiveTopology::LineStripWithAdjacency;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        return PrimitiveTopology::TriangleListWithAdjacency;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return PrimitiveTopology::TriangleStripWithAdjacency;

    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        return PrimitiveTopology::PatchList;

    default:
        return PrimitiveTopology::Undefined;
    }
}

} // namespace ogfx