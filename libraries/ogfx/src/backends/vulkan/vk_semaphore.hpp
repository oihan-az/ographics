#ifndef OGFX_VK_SEMAPHORE_HPP
#define OGFX_VK_SEMAPHORE_HPP

#include <ogfx/semaphore.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Semaphore::Impl
{
    VkSemaphore m_semaphore = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_SEMAPHORE_HPP