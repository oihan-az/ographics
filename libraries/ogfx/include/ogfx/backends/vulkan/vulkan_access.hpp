#ifndef OGFX_VULKAN_ACCESS_HPP
#define OGFX_VULKAN_ACCESS_HPP

#include <ogfx/command_buffer.hpp>
#include <ogfx/device.hpp>
#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/queue.hpp>
#include <ogfx/render_pass.hpp>

#include <vulkan/vulkan.h>

namespace ogfx::vulkan
{

class VulkanAccess
{
  public:
    static VkInstance instance(const Instance& instance);
    static VkPhysicalDevice physical_device(const PhysicalDevice& physical_device);
    static VkDevice device(const Device& device);
    static uint32_t queue_family_index(const Queue& queue);
    static VkQueue queue(const Queue& queue);
    static VkCommandBuffer command_buffer(const CommandBuffer& command_buffer);
    static VkRenderPass render_pass(const RenderPass& render_pass);
};

} // namespace ogfx::vulkan

#endif // OGFX_VULKAN_ACCESS_HPP