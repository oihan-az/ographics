#ifndef OGFX_VK_BUFFER_USAGE_HPP
#define OGFX_VK_BUFFER_USAGE_HPP

#include <ogfx/buffer_usage.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkBufferUsageFlags to_vk_buffer_usage(BufferUsage usage);
BufferUsage from_vk_buffer_usage(VkBufferUsageFlags usage);

} // namespace ogfx

#endif // OGFX_VK_BUFFER_USAGE_HPP