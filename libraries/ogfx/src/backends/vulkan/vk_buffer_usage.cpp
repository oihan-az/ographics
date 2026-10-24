#include "vk_buffer_usage.hpp"

namespace ogfx
{

VkBufferUsageFlags to_vk_buffer_usage(BufferUsage usage)
{
    VkBufferUsageFlags flags = 0;

    if (has_flag(usage, BufferUsage::Vertex))
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    if (has_flag(usage, BufferUsage::Index))
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (has_flag(usage, BufferUsage::Uniform))
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    if (has_flag(usage, BufferUsage::Storage))
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    if (has_flag(usage, BufferUsage::TransferSrc))
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    if (has_flag(usage, BufferUsage::TransferDst))
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (has_flag(usage, BufferUsage::Indirect))
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return flags;
}

BufferUsage from_vk_buffer_usage(VkBufferUsageFlags usage)
{
    BufferUsage result = BufferUsage::None;

    if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        result = result | BufferUsage::Vertex;

    if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
        result = result | BufferUsage::Index;

    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        result = result | BufferUsage::Uniform;

    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        result = result | BufferUsage::Storage;

    if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        result = result | BufferUsage::TransferSrc;

    if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        result = result | BufferUsage::TransferDst;

    if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
        result = result | BufferUsage::Indirect;

    return result;
}

} // namespace ogfx