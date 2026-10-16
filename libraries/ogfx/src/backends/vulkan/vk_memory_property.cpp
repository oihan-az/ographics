#include "vk_memory_property.hpp"

namespace ogfx
{

VkMemoryPropertyFlags to_vk_memory_property(MemoryProperty property)
{
    VkMemoryPropertyFlags flags = 0;

    if (has_flag(property, MemoryProperty::DeviceLocal))
    {
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    if (has_flag(property, MemoryProperty::HostVisible))
    {
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    if (has_flag(property, MemoryProperty::HostCoherent))
    {
        flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (has_flag(property, MemoryProperty::HostCached))
    {
        flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    return flags;
}

MemoryProperty from_vk_memory_property(VkMemoryPropertyFlags properties)
{
    MemoryProperty result = MemoryProperty::None;

    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        result = result | MemoryProperty::DeviceLocal;
    }

    if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        result = result | MemoryProperty::HostVisible;
    }

    if (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    {
        result = result | MemoryProperty::HostCoherent;
    }

    if (properties & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    {
        result = result | MemoryProperty::HostCached;
    }

    return result;
}

} // namespace ogfx