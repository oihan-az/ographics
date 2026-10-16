#ifndef OGFX_VK_MEMORY_PROPERTY_HPP
#define OGFX_VK_MEMORY_PROPERTY_HPP

#include <ogfx/memory_property.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkMemoryPropertyFlags to_vk_memory_property(MemoryProperty property);

MemoryProperty from_vk_memory_property(VkMemoryPropertyFlags properties);

} // namespace ogfx

#endif // OGFX_VK_MEMORY_PROPERTY_HPP