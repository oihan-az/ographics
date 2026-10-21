#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_image.hpp"
#include "vk_device.hpp"
#include "vk_physical_device.hpp"
#include "vk_format.hpp"
#include "vk_image_usage.hpp"

namespace ogfx
{

Image::Image(const PhysicalDevice& physical_device, const Device& device, const ImageDesc& desc)
    : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;
    m_impl->m_physical_device = physical_device.m_impl->m_physical_device;

    OGFX_LOG("Creating image:");
    OGFX_LOG("  Width: " + std::to_string(m_desc.width));
    OGFX_LOG("  Height: " + std::to_string(m_desc.height));
    OGFX_LOG("  Mip levels: " + std::to_string(m_desc.mip_levels));
    OGFX_LOG("  Array layers: " + std::to_string(m_desc.array_layers));
    OGFX_LOG("  Format: " + to_string_enum(m_desc.format));
    OGFX_LOG("  Usage: " + to_string_bitmask(m_desc.usage));
    OGFX_LOG("  Owned: " + std::string(m_desc.owned ? "true" : "false"));

    if (!m_desc.owned)
    {
        OGFX_LOG("Using externally owned Vulkan image");
        return;
    }

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = to_vk_format(m_desc.format);
    image_info.extent = {m_desc.width, m_desc.height, 1};
    image_info.mipLevels = m_desc.mip_levels;
    image_info.arrayLayers = m_desc.array_layers;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = to_vk_image_usage(m_desc.usage);
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(m_impl->m_device, &image_info, nullptr, &m_impl->m_image) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan image");
        throw std::runtime_error("Failed to create Vulkan image");
    }

    OGFX_LOG("Vulkan image created");
}

Image::~Image() 
{
    if (m_desc.owned && m_impl)
    {
        free_memory();

        if (m_impl->m_image != VK_NULL_HANDLE)
        {
            vkDestroyImage(m_impl->m_device, m_impl->m_image, nullptr);
            OGFX_LOG("Vulkan image destroyed");
        }
    }
}

Image::Image(Image&&) noexcept = default;
Image& Image::operator=(Image&&) noexcept = default;

void Image::allocate_memory()
{
    if (!m_impl || m_impl->m_image == VK_NULL_HANDLE)
    {
        OGFX_LOG("Cannot allocate memory: image is not initialized");
        return;
    }

    if (!m_desc.owned)
    {
        OGFX_LOG("Cannot allocate memory for an externally owned image");
        return;
    }

    free_memory();

    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(m_impl->m_device, m_impl->m_image, &memory_requirements);

    VkPhysicalDeviceMemoryProperties physical_memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(m_impl->m_physical_device, &physical_memory_properties);

    const VkMemoryPropertyFlags requested_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    uint32_t memory_type_index = physical_memory_properties.memoryTypeCount;

    for (uint32_t i = 0; i < physical_memory_properties.memoryTypeCount; ++i)
    {
        const bool type_supported = (memory_requirements.memoryTypeBits & (1u << i)) != 0;

        const VkMemoryPropertyFlags available_properties = physical_memory_properties.memoryTypes[i].propertyFlags;

        const bool properties_match = (available_properties & requested_properties) == requested_properties;

        if (type_supported && properties_match)
        {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == physical_memory_properties.memoryTypeCount)
    {
        OGFX_LOG("Failed to find compatible Vulkan image memory type");
        return;
    }

    VkMemoryAllocateInfo allocation_info{};
    allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation_info.allocationSize = memory_requirements.size;
    allocation_info.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(m_impl->m_device, &allocation_info, nullptr, &m_impl->m_memory) != VK_SUCCESS)
    {
        m_impl->m_memory = VK_NULL_HANDLE;
        OGFX_LOG("Failed to allocate Vulkan image memory");
        return;
    }

    OGFX_LOG("Vulkan image memory allocated:");
    OGFX_LOG("  Size: " + std::to_string(static_cast<uint64_t>(memory_requirements.size)));
}

void Image::free_memory()
{
    if (!m_impl || m_impl->m_memory == VK_NULL_HANDLE)
    {
        return;
    }

    vkFreeMemory(m_impl->m_device, m_impl->m_memory, nullptr);
    m_impl->m_memory = VK_NULL_HANDLE;

    OGFX_LOG("Vulkan image memory freed");
}

void Image::bind_memory()
{
    if (!m_impl || m_impl->m_image == VK_NULL_HANDLE || m_impl->m_memory == VK_NULL_HANDLE)
    {
        OGFX_LOG("Cannot bind Vulkan image memory: image or allocation is missing");
        return;
    }

    if (!m_desc.owned)
    {
        OGFX_LOG("Cannot bind memory for an externally owned image");
        return;
    }

    if (vkBindImageMemory(m_impl->m_device, m_impl->m_image, m_impl->m_memory, 0) != VK_SUCCESS)
    {
        OGFX_LOG("Failed to bind Vulkan image memory");
        return;
    }

    OGFX_LOG("Vulkan image memory bound");
}

} // namespace ogfx