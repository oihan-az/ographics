#ifndef OGFX_VK_RESOURCE_SET_HPP
#define OGFX_VK_RESOURCE_SET_HPP

#include <ogfx/resource_set.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct ResourceSet::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_RESOURCE_SET_HPP
