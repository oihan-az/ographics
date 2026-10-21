#ifndef OGFX_VK_RESOURCE_SET_LAYOUT_HPP
#define OGFX_VK_RESOURCE_SET_LAYOUT_HPP

#include <ogfx/resource_set_layout.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

class Device;

struct ResourceSetLayout::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_RESOURCE_SET_LAYOUT_HPP
