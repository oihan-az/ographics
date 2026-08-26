#ifndef OGFX_VK_INSTANCE_HPP
#define OGFX_VK_INSTANCE_HPP

#include <ogfx/instance.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Instance::Impl
{
    VkInstance m_instance = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_PHYSICAL_DEVICE_HPP