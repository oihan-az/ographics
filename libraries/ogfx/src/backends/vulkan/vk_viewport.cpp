#include "vk_viewport.hpp"

namespace ogfx
{

VkViewport to_vk_viewport(const Viewport& viewport)
{
    VkViewport result{};
    result.x = viewport.x;
    result.y = viewport.y;
    result.width = viewport.width;
    result.height = viewport.height;
    result.minDepth = viewport.min_depth;
    result.maxDepth = viewport.max_depth;
    return result;
}

Viewport from_vk_viewport(const VkViewport& viewport)
{
    Viewport result{};
    result.x = viewport.x;
    result.y = viewport.y;
    result.width = viewport.width;
    result.height = viewport.height;
    result.min_depth = viewport.minDepth;
    result.max_depth = viewport.maxDepth;
    return result;
}

VkRect2D to_vk_scissor(const Scissor& scissor)
{
    VkRect2D result{};
    result.offset.x = scissor.x;
    result.offset.y = scissor.y;
    result.extent.width = scissor.width;
    result.extent.height = scissor.height;
    return result;
}

Scissor from_vk_scissor(const VkRect2D& scissor)
{
    Scissor result{};
    result.x = scissor.offset.x;
    result.y = scissor.offset.y;
    result.width = scissor.extent.width;
    result.height = scissor.extent.height;
    return result;
}


} // namespace ogfx