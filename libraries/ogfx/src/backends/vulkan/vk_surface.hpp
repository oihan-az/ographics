#ifndef OGFX_VK_SURFACE_HPP
#define OGFX_VK_SURFACE_HPP

#include <ogfx/surface.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Surface::Impl
{
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_SURFACE_HPP