#ifndef OGFX_VK_PHYSICAL_DEVICE_HPP
#define OGFX_VK_PHYSICAL_DEVICE_HPP

#include <ogfx/physical_device.hpp>

#include "vk_queue_families.hpp"

#include <vulkan/vulkan.h>

namespace ogfx
{

class Surface;

struct PhysicalDevice::Impl
{
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;

    QueueFamilyIndices FindQueueFamilies(const Surface* surface = nullptr) const;
};

} // namespace ogfx

#endif // OGFX_VK_PHYSICAL_DEVICE_HPP