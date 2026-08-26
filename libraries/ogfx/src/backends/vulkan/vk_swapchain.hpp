#ifndef OGFX_VK_SWAPCHAIN_HPP
#define OGFX_VK_SWAPCHAIN_HPP

#include <ogfx/swapchain.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Swapchain::Impl
{
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_SWAPCHAIN_HPP