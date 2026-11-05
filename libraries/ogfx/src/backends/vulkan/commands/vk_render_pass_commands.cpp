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
    OGFX_LOG("  Clear values:");
    OGFX_LOG("    Count: " + std::to_string(begin_info.clear_values.size()));

    VkRenderPassBeginInfo vk_begin_info{};
    vk_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vk_begin_info.renderPass = begin_info.render_pass->m_impl->m_render_pass;
    vk_begin_info.framebuffer = begin_info.framebuffer->m_impl->m_framebuffer;

    vk_begin_info.renderArea.offset = {static_cast<int32_t>(begin_info.offset_x),
                                       static_cast<int32_t>(begin_info.offset_y)};

    vk_begin_info.renderArea.extent = {begin_info.width, begin_info.height};

   std::vector<VkClearValue> vk_clear_values;
    vk_clear_values.reserve(begin_info.clear_values.size());
    for (size_t i = 0; i < begin_info.clear_values.size(); ++i)
    {
        const ClearValue& clear_value = begin_info.clear_values[i];

        VkClearValue vk_clear_value{};

        switch (clear_value.type)
        {
        case ClearValueType::Color:
        {
            vk_clear_value.color = {clear_value.color.r, clear_value.color.g, clear_value.color.b, clear_value.color.a};

            OGFX_LOG("  Clear value [" + std::to_string(i) + "] - Color:");
            OGFX_LOG("    R: " + std::to_string(clear_value.color.r));
            OGFX_LOG("    G: " + std::to_string(clear_value.color.g));
            OGFX_LOG("    B: " + std::to_string(clear_value.color.b));
            OGFX_LOG("    A: " + std::to_string(clear_value.color.a));
            break;
        }

        case ClearValueType::DepthStencil:
        {
            vk_clear_value.depthStencil = {clear_value.depth, clear_value.stencil};

            OGFX_LOG("  Clear value [" + std::to_string(i) + "] - Depth/Stencil:");
            OGFX_LOG("    Depth: " + std::to_string(clear_value.depth));
            OGFX_LOG("    Stencil: " + std::to_string(clear_value.stencil));
            break;
        }
        }

        vk_clear_values.push_back(vk_clear_value);
    }
    vk_begin_info.clearValueCount = static_cast<uint32_t>(vk_clear_values.size());
    vk_begin_info.pClearValues = vk_clear_values.empty() ? nullptr : vk_clear_values.data();

    vkCmdBeginRenderPass(m_impl->m_command_buffer, &vk_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    OGFX_LOG("Vulkan render pass begun");
}

void CommandBuffer::end_render_pass()
{
    vkCmdEndRenderPass(m_impl->m_command_buffer);

    OGFX_LOG("Vulkan render pass ended");
}

} // namespace ogfx