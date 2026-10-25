#include <debug.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/render_pass_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_framebuffer.hpp"
#include "../vk_render_pass.hpp"

namespace ogfx
{

void CommandBuffer::begin_render_pass(const RenderPassBeginInfo& begin_info)
{
    OGFX_LOG("Beginning Vulkan render pass");

    OGFX_LOG("  Render area:");
    OGFX_LOG("    Offset: (" + std::to_string(begin_info.offset_x) + ", " + std::to_string(begin_info.offset_y) + ")");
    OGFX_LOG("    Extent: " + std::to_string(begin_info.width) + "x" + std::to_string(begin_info.height));

    OGFX_LOG("  Clear color:");
    OGFX_LOG("    R: " + std::to_string(begin_info.clear_color.r));
    OGFX_LOG("    G: " + std::to_string(begin_info.clear_color.g));
    OGFX_LOG("    B: " + std::to_string(begin_info.clear_color.b));
    OGFX_LOG("    A: " + std::to_string(begin_info.clear_color.a));

    VkRenderPassBeginInfo vk_begin_info{};
    vk_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vk_begin_info.renderPass = begin_info.render_pass->m_impl->m_render_pass;
    vk_begin_info.framebuffer = begin_info.framebuffer->m_impl->m_framebuffer;

    vk_begin_info.renderArea.offset = {static_cast<int32_t>(begin_info.offset_x),
                                       static_cast<int32_t>(begin_info.offset_y)};

    vk_begin_info.renderArea.extent = {begin_info.width, begin_info.height};

    VkClearValue clear_value{};
    clear_value.color = {begin_info.clear_color.r, begin_info.clear_color.g, begin_info.clear_color.b,
                         begin_info.clear_color.a};

    vk_begin_info.clearValueCount = 1;
    vk_begin_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(m_impl->m_command_buffer, &vk_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    OGFX_LOG("Vulkan render pass begun");
}

void CommandBuffer::end_render_pass()
{
    vkCmdEndRenderPass(m_impl->m_command_buffer);

    OGFX_LOG("Vulkan render pass ended");
}

} // namespace ogfx