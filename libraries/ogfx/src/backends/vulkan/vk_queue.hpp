#ifndef OGFX_VK_QUEUE_HPP
#define OGFX_VK_QUEUE_HPP

#include <ogfx/queue.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ogfx
{

struct Queue::Impl
{
    VkQueue m_queue = VK_NULL_HANDLE;
    uint32_t m_family_index = UINT32_MAX;
};

} // namespace ogfx

#endif // OGFX_VK_QUEUE_HPP