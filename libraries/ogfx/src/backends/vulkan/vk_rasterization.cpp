#include "vk_rasterization.hpp"

namespace ogfx
{

VkPolygonMode to_vk_polygon_mode(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case PolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case PolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    case PolygonMode::Undefined:
        return VK_POLYGON_MODE_MAX_ENUM;
    }

    return VK_POLYGON_MODE_MAX_ENUM;
}

PolygonMode from_vk_polygon_mode(VkPolygonMode mode)
{
    switch (mode)
    {
    case VK_POLYGON_MODE_FILL:
        return PolygonMode::Fill;
    case VK_POLYGON_MODE_LINE:
        return PolygonMode::Line;
    case VK_POLYGON_MODE_POINT:
        return PolygonMode::Point;
    default:
        return PolygonMode::Undefined;
    }
}

VkCullModeFlags to_vk_cull_mode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:
        return VK_CULL_MODE_NONE;
    case CullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case CullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    case CullMode::FrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
    case CullMode::Undefined:
        return 0;
    }

    return 0;
}

CullMode from_vk_cull_mode(VkCullModeFlags mode)
{
    switch (mode)
    {
    case VK_CULL_MODE_NONE:
        return CullMode::None;
    case VK_CULL_MODE_FRONT_BIT:
        return CullMode::Front;
    case VK_CULL_MODE_BACK_BIT:
        return CullMode::Back;
    case VK_CULL_MODE_FRONT_AND_BACK:
        return CullMode::FrontAndBack;
    default:
        return CullMode::Undefined;
    }
}

VkFrontFace to_vk_front_face(FrontFace face)
{
    switch (face)
    {
    case FrontFace::CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case FrontFace::Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;
    case FrontFace::Undefined:
        return VK_FRONT_FACE_MAX_ENUM;
    }

    return VK_FRONT_FACE_MAX_ENUM;
}

FrontFace from_vk_front_face(VkFrontFace face)
{
    switch (face)
    {
    case VK_FRONT_FACE_COUNTER_CLOCKWISE:
        return FrontFace::CounterClockwise;
    case VK_FRONT_FACE_CLOCKWISE:
        return FrontFace::Clockwise;
    default:
        return FrontFace::Undefined;
    }
}

} // namespace ogfx