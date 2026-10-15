#ifndef OGFX_VK_VIEWPORT_HPP
#define OGFX_VK_VIEWPORT_HPP

#include <ogfx/viewport.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkViewport to_vk_viewport(const Viewport& viewport);
Viewport from_vk_viewport(const VkViewport& viewport);

VkRect2D to_vk_scissor(const Scissor& scissor);
Scissor from_vk_scissor(const VkRect2D& scissor);

} // namespace ogfx

#endif // OGFX_VK_VIEWPORT_HPP