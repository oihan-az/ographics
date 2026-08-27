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
};

} // namespace ogfx

#endif // OGFX_VK_PHYSICAL_DEVICE_HPP