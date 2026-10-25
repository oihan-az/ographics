#include <debug.hpp>

#include "vk_command_buffer.hpp"
#include "vk_command_pool.hpp"

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

} // namespace ogfx