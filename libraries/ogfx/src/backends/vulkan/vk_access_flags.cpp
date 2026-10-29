#include "vk_access_flags.hpp"

namespace ogfx
{

VkAccessFlags to_vk_access_flags(AccessFlags flags)
{
    VkAccessFlags vk_flags = 0;

    if (has_flag(flags, AccessFlags::IndirectCommandRead))
        vk_flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

    if (has_flag(flags, AccessFlags::IndexRead))
        vk_flags |= VK_ACCESS_INDEX_READ_BIT;

    if (has_flag(flags, AccessFlags::VertexAttributeRead))
        vk_flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    if (has_flag(flags, AccessFlags::UniformRead))
        vk_flags |= VK_ACCESS_UNIFORM_READ_BIT;

    if (has_flag(flags, AccessFlags::ShaderRead))
        vk_flags |= VK_ACCESS_SHADER_READ_BIT;

    if (has_flag(flags, AccessFlags::ShaderWrite))
        vk_flags |= VK_ACCESS_SHADER_WRITE_BIT;

    if (has_flag(flags, AccessFlags::ColorAttachmentRead))
        vk_flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    if (has_flag(flags, AccessFlags::ColorAttachmentWrite))
        vk_flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    if (has_flag(flags, AccessFlags::DepthStencilAttachmentRead))
        vk_flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    if (has_flag(flags, AccessFlags::DepthStencilAttachmentWrite))
        vk_flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (has_flag(flags, AccessFlags::TransferRead))
        vk_flags |= VK_ACCESS_TRANSFER_READ_BIT;

    if (has_flag(flags, AccessFlags::TransferWrite))
        vk_flags |= VK_ACCESS_TRANSFER_WRITE_BIT;

    if (has_flag(flags, AccessFlags::HostRead))
        vk_flags |= VK_ACCESS_HOST_READ_BIT;

    if (has_flag(flags, AccessFlags::HostWrite))
        vk_flags |= VK_ACCESS_HOST_WRITE_BIT;

    if (has_flag(flags, AccessFlags::MemoryRead))
        vk_flags |= VK_ACCESS_MEMORY_READ_BIT;

    if (has_flag(flags, AccessFlags::MemoryWrite))
        vk_flags |= VK_ACCESS_MEMORY_WRITE_BIT;

    if (has_flag(flags, AccessFlags::AccelerationStructureRead))
        vk_flags |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    if (has_flag(flags, AccessFlags::AccelerationStructureWrite))
        vk_flags |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

    return vk_flags;
}

AccessFlags from_vk_access_flags(VkAccessFlags flags)
{
    AccessFlags result = AccessFlags::None;

    if (flags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT)
        result |= AccessFlags::IndirectCommandRead;

    if (flags & VK_ACCESS_INDEX_READ_BIT)
        result |= AccessFlags::IndexRead;

    if (flags & VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)
        result |= AccessFlags::VertexAttributeRead;

    if (flags & VK_ACCESS_UNIFORM_READ_BIT)
        result |= AccessFlags::UniformRead;

    if (flags & VK_ACCESS_SHADER_READ_BIT)
        result |= AccessFlags::ShaderRead;

    if (flags & VK_ACCESS_SHADER_WRITE_BIT)
        result |= AccessFlags::ShaderWrite;

    if (flags & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
        result |= AccessFlags::ColorAttachmentRead;

    if (flags & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
        result |= AccessFlags::ColorAttachmentWrite;

    if (flags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
        result |= AccessFlags::DepthStencilAttachmentRead;

    if (flags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
        result |= AccessFlags::DepthStencilAttachmentWrite;

    if (flags & VK_ACCESS_TRANSFER_READ_BIT)
        result |= AccessFlags::TransferRead;

    if (flags & VK_ACCESS_TRANSFER_WRITE_BIT)
        result |= AccessFlags::TransferWrite;

    if (flags & VK_ACCESS_HOST_READ_BIT)
        result |= AccessFlags::HostRead;

    if (flags & VK_ACCESS_HOST_WRITE_BIT)
        result |= AccessFlags::HostWrite;

    if (flags & VK_ACCESS_MEMORY_READ_BIT)
        result |= AccessFlags::MemoryRead;

    if (flags & VK_ACCESS_MEMORY_WRITE_BIT)
        result |= AccessFlags::MemoryWrite;

    if (flags & VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR)
        result |= AccessFlags::AccelerationStructureRead;

    if (flags & VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)
        result |= AccessFlags::AccelerationStructureWrite;

    return result;
}

} // namespace ogfx