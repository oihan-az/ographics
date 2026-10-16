#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_buffer.hpp"
#include "vk_buffer_usage.hpp"
#include "vk_memory_property.hpp"
#include "vk_physical_device.hpp"
#include "vk_device.hpp"

#include <stdexcept>

namespace ogfx
{

Buffer::Buffer(const PhysicalDevice& physical_device, const Device& device, const BufferDesc& desc)
    : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;
    m_impl->m_physical_device = physical_device.m_impl->m_physical_device;

    create_buffer();
}

Buffer::~Buffer()
{
    destroy_buffer();
    free_memory();
}

Buffer::Buffer(Buffer&&) noexcept = default;

Buffer& Buffer::operator=(Buffer&&) noexcept = default;

void Buffer::create_buffer()
{
    OGFX_LOG("Creating buffer: size " + std::to_string(m_desc.size) + 
             ", usage " + to_string_bitmask(m_desc.usage) +
             ", memory properties " + to_string_bitmask(m_desc.memory_properties));

    // Destroy any existing resources before recreating the buffer
    destroy_buffer();
    free_memory();

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = static_cast<VkDeviceSize>(m_desc.size);
    buffer_info.usage = to_vk_buffer_usage(m_desc.usage);
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_impl->m_device, &buffer_info, nullptr, &m_impl->m_buffer) != VK_SUCCESS)
    {
        m_impl->m_buffer = VK_NULL_HANDLE;
        OGFX_LOG_ERROR("Failed to create Vulkan buffer");
        throw std::runtime_error("Failed to create Vulkan buffer");
    }

    OGFX_LOG("Vulkan buffer created");
}

void Buffer::destroy_buffer()
{
    if (!m_impl || m_impl->m_buffer == VK_NULL_HANDLE)
    {
        return;
    }

    vkDestroyBuffer(m_impl->m_device, m_impl->m_buffer, nullptr);

    m_impl->m_buffer = VK_NULL_HANDLE;

    OGFX_LOG("Vulkan buffer destroyed");
}

void Buffer::allocate_memory()
{
    if (!m_impl || m_impl->m_buffer == VK_NULL_HANDLE)
    {
        OGFX_LOG("Cannot allocate memory: buffer is not initialized");
        return;
    }

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(m_impl->m_device, m_impl->m_buffer, &memory_requirements);

    VkPhysicalDeviceMemoryProperties physical_memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(m_impl->m_physical_device, &physical_memory_properties);

    const VkMemoryPropertyFlags requested_properties = to_vk_memory_property(m_desc.memory_properties);

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
        OGFX_LOG("Failed to find compatible Vulkan memory type. Requested: " + to_string_bitmask(m_desc.memory_properties));
        return;
    }

    VkMemoryAllocateInfo allocation_info{};
    allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation_info.allocationSize = memory_requirements.size;
    allocation_info.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(m_impl->m_device, &allocation_info, nullptr, &m_impl->m_memory) != VK_SUCCESS)
    {
        m_impl->m_memory = VK_NULL_HANDLE;
        OGFX_LOG("Failed to allocate Vulkan buffer memory.");
        return;
    }

    m_impl->m_memory_property_flags = physical_memory_properties.memoryTypes[memory_type_index].propertyFlags;

    OGFX_LOG("Vulkan buffer memory allocated: size " +
             std::to_string(static_cast<uint64_t>(memory_requirements.size)));
}

void Buffer::free_memory()
{
    if (!m_impl || m_impl->m_memory == VK_NULL_HANDLE)
    {
        return;
    }

    vkFreeMemory(m_impl->m_device, m_impl->m_memory, nullptr);

    m_impl->m_memory = VK_NULL_HANDLE;
    m_impl->m_memory_property_flags = 0;

    OGFX_LOG("Vulkan buffer memory freed");
}

void Buffer::bind_memory()
{
    if (!m_impl || m_impl->m_buffer == VK_NULL_HANDLE || m_impl->m_memory == VK_NULL_HANDLE)
    {
        OGFX_LOG("Cannot bind Vulkan buffer memory: buffer or allocation is missing");
        return;
    }

    if (vkBindBufferMemory(m_impl->m_device, m_impl->m_buffer, m_impl->m_memory, 0) != VK_SUCCESS)
    {
        OGFX_LOG("Failed to bind Vulkan buffer memory");
        return;
    }

    OGFX_LOG("Vulkan buffer memory bound");
}

void Buffer::write(const BufferWriteInfo& info)
{
    if (!m_impl || m_impl->m_buffer == VK_NULL_HANDLE || m_impl->m_memory == VK_NULL_HANDLE)
    {
        OGFX_LOG_ERROR("Cannot write buffer: buffer or memory is missing");
        return;
    }

    if (info.size == 0)
    {
        return;
    }

    if (!info.data)
    {
        OGFX_LOG_ERROR("Cannot write buffer: source data is null");
        return;
    }

    if (info.offset > m_desc.size || info.size > m_desc.size - info.offset)
    {
        OGFX_LOG_ERROR("Cannot write buffer: write range exceeds buffer size");
        return;
    }

    if ((m_impl->m_memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
    {
        OGFX_LOG_ERROR("Cannot write buffer: allocated memory is not host-visible");
        return;
    }

    void* mapped_memory = nullptr;
    if (vkMapMemory(m_impl->m_device, m_impl->m_memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to map Vulkan buffer memory");
        return;
    }

    auto* destination = static_cast<uint8_t*>(mapped_memory) + info.offset;
    std::memcpy(destination, info.data, static_cast<size_t>(info.size));

    const bool host_coherent = (m_impl->m_memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
    if (!host_coherent)
    {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = m_impl->m_memory;
        range.offset = 0;
        range.size = VK_WHOLE_SIZE;

        if (vkFlushMappedMemoryRanges(m_impl->m_device, 1, &range) != VK_SUCCESS)
        {
            OGFX_LOG_ERROR("Failed to flush Vulkan buffer memory");
            vkUnmapMemory(m_impl->m_device, m_impl->m_memory);
            return;
        }
    }

    vkUnmapMemory(m_impl->m_device, m_impl->m_memory);

    OGFX_LOG("Wrote buffer data: size " + std::to_string(info.size) + ", offset " + std::to_string(info.offset));
}

} // namespace ogfx