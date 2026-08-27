#ifndef OGFX_VK_COMMAND_POOL_HPP
#define OGFX_VK_COMMAND_POOL_HPP

#include <ogfx/command_pool.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct CommandPool::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_command_pool = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_COMMAND_POOL_HPP