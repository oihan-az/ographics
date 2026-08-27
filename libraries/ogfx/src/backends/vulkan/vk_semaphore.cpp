#include <debug.hpp>

#include "vk_semaphore.hpp"
#include "vk_device.hpp"

#include <stdexcept>

namespace ogfx
{

Semaphore::Semaphore(const Device& device) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    VkSemaphoreCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    OGFX_LOG("Creating Vulkan semaphore");

    if (vkCreateSemaphore(m_impl->m_device, &create_info, nullptr, &m_impl->m_semaphore) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan semaphore");
        throw std::runtime_error("Failed to create Vulkan semaphore.");
    }

    OGFX_LOG("Vulkan semaphore created");
}

Semaphore::~Semaphore()
{
    if (m_impl && m_impl->m_semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_impl->m_device, m_impl->m_semaphore, nullptr);

        OGFX_LOG("Vulkan semaphore destroyed");
    }
}

Semaphore::Semaphore(Semaphore&&) noexcept = default;

Semaphore& Semaphore::operator=(Semaphore&&) noexcept = default;

} // namespace ogfx