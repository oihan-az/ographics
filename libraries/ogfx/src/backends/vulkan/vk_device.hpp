#ifndef OGFX_VKL_DEVICE_HPP
#define OGFX_VKL_DEVICE_HPP

#include <ogfx/device.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

class Surface;

struct Device::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;

    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_PHYSICAL_DEVICE_HPP