#ifndef OGFX_VK_BUFFER_HPP
#define OGFX_VK_BUFFER_HPP

#include <ogfx/buffer.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Buffer::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;

    VkMemoryPropertyFlags m_memory_property_flags = 0;
};

} // namespace ogfx

#endif // OGFX_VK_BUFFER_HPP