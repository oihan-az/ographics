#include <debug.hpp>

#include "vk_command_buffer.hpp"
#include "vk_command_pool.hpp"
#include "vk_render_pass.hpp"
#include "vk_framebuffer.hpp"

#include <stdexcept>

namespace ogfx
{

CommandBuffer::CommandBuffer(const CommandPool& command_pool) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = command_pool.m_impl->m_device;
    m_impl->m_command_pool = command_pool.m_impl->m_command_pool;

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = m_impl->m_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    OGFX_LOG("Allocating Vulkan command buffer");

    if (vkAllocateCommandBuffers(m_impl->m_device, &allocate_info, &m_impl->m_command_buffer) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to allocate Vulkan command buffer");
        throw std::runtime_error("Failed to allocate Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer allocated");
}

CommandBuffer::~CommandBuffer()
{
    if (m_impl && m_impl->m_command_buffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_impl->m_device, m_impl->m_command_pool, 1, &m_impl->m_command_buffer);

        OGFX_LOG("Vulkan command buffer freed");
    }
}

CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;

CommandBuffer& CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

void CommandBuffer::begin()
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_impl->m_command_buffer, &begin_info) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to begin Vulkan command buffer");
        throw std::runtime_error("Failed to begin Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer recording started");
}

void CommandBuffer::end()
{
    if (vkEndCommandBuffer(m_impl->m_command_buffer) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to end Vulkan command buffer");
        throw std::runtime_error("Failed to end Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer recording ended");
}

void CommandBuffer::reset()
{
    if (vkResetCommandBuffer(m_impl->m_command_buffer, 0) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to reset Vulkan command buffer");
        throw std::runtime_error("Failed to reset Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer reset");
}

void CommandBuffer::begin_render_pass(const RenderPass& render_pass, const Framebuffer& framebuffer,
                                      const RenderPassBeginInfo& begin_info)
{
    OGFX_LOG("Beginning Vulkan render pass");

    OGFX_LOG("  Render area: offset=(" + std::to_string(begin_info.offset_x) + ", " +
             std::to_string(begin_info.offset_y) + "), extent=(" + std::to_string(begin_info.width) + "x" +
             std::to_string(begin_info.height) + ")");

    OGFX_LOG("  Clear color: (" + std::to_string(begin_info.clear_color.r) + ", " +
             std::to_string(begin_info.clear_color.g) + ", " + std::to_string(begin_info.clear_color.b) + ", " +
             std::to_string(begin_info.clear_color.a) + ")");

    VkRenderPassBeginInfo vk_begin_info{};
    vk_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vk_begin_info.renderPass = render_pass.m_impl->m_render_pass;
    vk_begin_info.framebuffer = framebuffer.m_impl->m_framebuffer;

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