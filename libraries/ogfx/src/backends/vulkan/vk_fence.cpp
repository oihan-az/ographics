#include <debug.hpp>

#include "vk_fence.hpp"
#include "vk_device.hpp"

#include <stdexcept>

namespace ogfx
{

Fence::Fence(const Device& device, bool signaled) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    VkFenceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (signaled)
    {
        create_info.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    }

    OGFX_LOG("Creating Vulkan fence (" + std::string(signaled ? "signaled" : "unsignaled") + ")");

    if (vkCreateFence(m_impl->m_device, &create_info, nullptr, &m_impl->m_fence) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan fence");
        throw std::runtime_error("Failed to create Vulkan fence.");
    }

    OGFX_LOG("Vulkan fence created");
}

Fence::~Fence()
{
    if (m_impl && m_impl->m_fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_impl->m_device, m_impl->m_fence, nullptr);

        OGFX_LOG("Vulkan fence destroyed");
    }
}

Fence::Fence(Fence&&) noexcept = default;

Fence& Fence::operator=(Fence&&) noexcept = default;

void Fence::wait()
{
    OGFX_LOG("Waiting for fence");

    if (vkWaitForFences(m_impl->m_device, 1, &m_impl->m_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to wait for Vulkan fence.");
    }

    OGFX_LOG("Fence signaled");
}

void Fence::reset()
{
    if (vkResetFences(m_impl->m_device, 1, &m_impl->m_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset Vulkan fence.");
    }

    OGFX_LOG("Fence reset");
}

} // namespace ogfx