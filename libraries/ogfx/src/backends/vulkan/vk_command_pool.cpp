#include <debug.hpp>

#include "vk_command_pool.hpp"
#include "vk_device.hpp"
#include "vk_queue.hpp"

#include <stdexcept>

namespace ogfx
{

CommandPool::CommandPool(const Device& device, const Queue& queue, const CommandPoolDesc& desc)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue.m_impl->m_family_index;

    if (desc.transient)
    {
        create_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }

    if (desc.resettable)
    {
        create_info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    OGFX_LOG("Creating Vulkan command pool");

    if (vkCreateCommandPool(m_impl->m_device, &create_info, nullptr, &m_impl->m_command_pool) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan command pool");
        throw std::runtime_error("Failed to create Vulkan command pool.");
    }

    OGFX_LOG("Vulkan command pool created");
}

CommandPool::~CommandPool()
{
    if (m_impl && m_impl->m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_impl->m_device, m_impl->m_command_pool, nullptr);

        OGFX_LOG("Vulkan command pool destroyed");
    }
}

CommandPool::CommandPool(CommandPool&&) noexcept = default;

CommandPool& CommandPool::operator=(CommandPool&&) noexcept = default;

} // namespace ogfx