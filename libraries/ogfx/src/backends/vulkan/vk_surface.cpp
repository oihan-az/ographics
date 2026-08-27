#include <debug.hpp>

#include "vk_surface.hpp"
#include "vk_instance.hpp"

#include <stdexcept>

namespace ogfx
{

Surface::Surface(const Instance& instance, NativeSurfaceHandle handle) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_surface = reinterpret_cast<VkSurfaceKHR>(handle);
    m_impl->m_instance = instance.m_impl->m_instance;
}

Surface::~Surface() 
{
    if (m_impl && m_impl->m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_impl->m_instance, m_impl->m_surface, nullptr);

        OGFX_LOG("Vulkan surface destroyed");
    }
}

Surface::Surface(Surface&&) noexcept = default;
Surface& Surface::operator=(Surface&&) noexcept = default;

} // namespace ogfx