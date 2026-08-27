#ifndef OGFX_VK_COMMAND_BUFFER_HPP
#define OGFX_VK_COMMAND_BUFFER_HPP

#include <ogfx/command_buffer.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct CommandBuffer::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_command_pool = VK_NULL_HANDLE;
    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_COMMAND_BUFFER_HPP