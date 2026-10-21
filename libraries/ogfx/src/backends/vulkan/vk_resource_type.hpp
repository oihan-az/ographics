#ifndef OGFX_VK_RESOURCE_TYPE_HPP
#define OGFX_VK_RESOURCE_TYPE_HPP

#include <ogfx/resource_type.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkDescriptorType to_vk_descriptor_type(ResourceType type);
ResourceType from_vk_descriptor_type(VkDescriptorType type);

} // namespace ogfx

#endif // OGFX_VK_RESOURCE_TYPE_HPP
