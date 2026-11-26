#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"

namespace ogfx
{

void CommandBuffer::set_viewport(const Viewport& viewport)
{
    VkViewport vk_viewport{};
    vk_viewport.x = viewport.x;
    vk_viewport.y = viewport.y;
    vk_viewport.width = viewport.width;
    vk_viewport.height = viewport.height;
    vk_viewport.minDepth = viewport.min_depth;
    vk_viewport.maxDepth = viewport.max_depth;

    vkCmdSetViewport(m_impl->m_command_buffer, 0, 1, &vk_viewport);
}

void CommandBuffer::set_scissor(const Scissor& scissor)
{
    VkRect2D vk_scissor{};
    vk_scissor.offset.x = scissor.x;
    vk_scissor.offset.y = scissor.y;
    vk_scissor.extent.width = scissor.width;
    vk_scissor.extent.height = scissor.height;

    vkCmdSetScissor(m_impl->m_command_buffer, 0, 1, &vk_scissor);
}

} // namespace ogfx