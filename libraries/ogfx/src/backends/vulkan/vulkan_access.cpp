#include <ogfx/backends/vulkan/vulkan_access.hpp>

#include "vk_command_buffer.hpp"
#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_physical_device.hpp"
#include "vk_queue.hpp"
#include "vk_render_pass.hpp"

namespace ogfx::vulkan
{

VkInstance VulkanAccess::instance(const Instance& instance)
{
    return instance.m_impl->m_instance;
}

VkPhysicalDevice VulkanAccess::physical_device(const PhysicalDevice& physical_device)
{
    return physical_device.m_impl->m_physical_device;
}

VkDevice VulkanAccess::device(const Device& device)
{
    return device.m_impl->m_device;
}

uint32_t VulkanAccess::queue_family_index(const Queue& queue)
{
    return queue.m_impl->m_family_index;
}

VkQueue VulkanAccess::queue(const Queue& queue)
{
    return queue.m_impl->m_queue;
}

VkCommandBuffer VulkanAccess::command_buffer(const CommandBuffer& command_buffer)
{
    return command_buffer.m_impl->m_command_buffer;
}

VkRenderPass VulkanAccess::render_pass(const RenderPass& render_pass)
{
    return render_pass.m_impl->m_render_pass;
}

} // namespace ogfx::vulkan