#ifndef OGFX_VK_FENCE_HPP
#define OGFX_VK_FENCE_HPP

#include <ogfx/fence.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{


struct Fence::Impl
{
    VkFence m_fence = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_FENCE_HPP