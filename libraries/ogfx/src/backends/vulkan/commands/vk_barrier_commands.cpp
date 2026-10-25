#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/barrier_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_pipeline_stage.hpp"
#include "../vk_access_flags.hpp"
#include "../vk_image_state.hpp"
#include "../vk_buffer.hpp"
#include "../vk_image.hpp"

namespace ogfx
{

void CommandBuffer::buffer_barrier(const BufferBarrierInfo& info)
{
    OGFX_LOG("Recording buffer barrier:");
    OGFX_LOG("  Offset: " + std::to_string(info.offset));
    OGFX_LOG("  Size: " + std::to_string(info.size));
    OGFX_LOG("  Source stages: " + to_string_bitmask(info.memory_barrier_info.src_stages));
    OGFX_LOG("  Destination stages: " + to_string_bitmask(info.memory_barrier_info.dst_stages));
    OGFX_LOG("  Source access: " + to_string_bitmask(info.memory_barrier_info.src_access));
    OGFX_LOG("  Destination access: " + to_string_bitmask(info.memory_barrier_info.dst_access));

    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = to_vk_access_flags(info.memory_barrier_info.src_access);
    barrier.dstAccessMask = to_vk_access_flags(info.memory_barrier_info.dst_access);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = info.buffer->m_impl->m_buffer;
    barrier.offset = static_cast<VkDeviceSize>(info.offset);
    barrier.size = static_cast<VkDeviceSize>(info.size);

    vkCmdPipelineBarrier(m_impl->m_command_buffer, to_vk_pipeline_stage(info.memory_barrier_info.src_stages),
                         to_vk_pipeline_stage(info.memory_barrier_info.dst_stages), 0, 0, nullptr, 1, &barrier, 0,
                         nullptr);

    OGFX_LOG("Buffer barrier recorded");
}

void CommandBuffer::image_barrier(const ImageBarrierInfo& info)
{
    OGFX_LOG("Recording image barrier:");
    OGFX_LOG("  Old state: " + to_string_enum(info.old_state));
    OGFX_LOG("  New state: " + to_string_enum(info.new_state));
    OGFX_LOG("  Source stages: " + to_string_bitmask(info.memory_barrier_info.src_stages));
    OGFX_LOG("  Destination stages: " + to_string_bitmask(info.memory_barrier_info.dst_stages));
    OGFX_LOG("  Source access: " + to_string_bitmask(info.memory_barrier_info.src_access));
    OGFX_LOG("  Destination access: " + to_string_bitmask(info.memory_barrier_info.dst_access));
    OGFX_LOG("  Base mip level: " + std::to_string(info.base_mip_level));
    OGFX_LOG("  Mip level count: " + std::to_string(info.mip_level_count));
    OGFX_LOG("  Base array layer: " + std::to_string(info.base_array_layer));
    OGFX_LOG("  Array layer count: " + std::to_string(info.array_layer_count));

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = to_vk_access_flags(info.memory_barrier_info.src_access);
    barrier.dstAccessMask = to_vk_access_flags(info.memory_barrier_info.dst_access);
    barrier.oldLayout = to_vk_image_state(info.old_state);
    barrier.newLayout = to_vk_image_state(info.new_state);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = info.image->m_impl->m_image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = info.base_mip_level;
    barrier.subresourceRange.levelCount = info.mip_level_count;
    barrier.subresourceRange.baseArrayLayer = info.base_array_layer;
    barrier.subresourceRange.layerCount = info.array_layer_count;

    vkCmdPipelineBarrier(m_impl->m_command_buffer, to_vk_pipeline_stage(info.memory_barrier_info.src_stages),
                         to_vk_pipeline_stage(info.memory_barrier_info.dst_stages), 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    OGFX_LOG("Image barrier recorded");
}

struct ImageStateSyncInfo
{
    PipelineStage stages = PipelineStage::AllCommands;
    AccessFlags access = AccessFlags::MemoryRead | AccessFlags::MemoryWrite;
};

ImageStateSyncInfo get_image_state_src_info(ImageState state)
{
    switch (state)
    {
    case ImageState::Undefined:
        return {PipelineStage::TopOfPipe, AccessFlags::None};

    case ImageState::Common:
        return {PipelineStage::AllCommands, AccessFlags::MemoryRead | AccessFlags::MemoryWrite};

    case ImageState::RenderTarget:
        return {PipelineStage::ColorAttachmentOutput,
                AccessFlags::ColorAttachmentRead | AccessFlags::ColorAttachmentWrite};

    case ImageState::DepthStencil:
        return {PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests,
                AccessFlags::DepthStencilAttachmentRead | AccessFlags::DepthStencilAttachmentWrite};

    case ImageState::ShaderRead:
        return {PipelineStage::VertexShader | PipelineStage::FragmentShader | PipelineStage::ComputeShader,
                AccessFlags::ShaderRead};

    case ImageState::TransferSrc:
        return {PipelineStage::Transfer, AccessFlags::TransferRead};

    case ImageState::TransferDst:
        return {PipelineStage::Transfer, AccessFlags::TransferWrite};

    case ImageState::Present:
        return {PipelineStage::AllCommands, AccessFlags::MemoryRead};
    }

    return {};
}

ImageStateSyncInfo get_image_state_dst_info(ImageState state)
{
    switch (state)
    {
    case ImageState::Undefined:
        return {PipelineStage::TopOfPipe, AccessFlags::None};

    case ImageState::Common:
        return {PipelineStage::AllCommands, AccessFlags::MemoryRead | AccessFlags::MemoryWrite};

    case ImageState::RenderTarget:
        return {PipelineStage::ColorAttachmentOutput,
                AccessFlags::ColorAttachmentRead | AccessFlags::ColorAttachmentWrite};

    case ImageState::DepthStencil:
        return {PipelineStage::EarlyFragmentTests | PipelineStage::LateFragmentTests,
                AccessFlags::DepthStencilAttachmentRead | AccessFlags::DepthStencilAttachmentWrite};

    case ImageState::ShaderRead:
        return {PipelineStage::VertexShader | PipelineStage::FragmentShader | PipelineStage::ComputeShader,
                AccessFlags::ShaderRead};

    case ImageState::TransferSrc:
        return {PipelineStage::Transfer, AccessFlags::TransferRead};

    case ImageState::TransferDst:
        return {PipelineStage::Transfer, AccessFlags::TransferWrite};

    case ImageState::Present:
        return {PipelineStage::BottomOfPipe, AccessFlags::None};
    }

    return {};
}

void CommandBuffer::transition_image(const ImageTransitionInfo& info)
{
    if (info.new_state == ImageState::Undefined)
    {
        OGFX_LOG_ERROR("Cannot transition image to Undefined state");
        return;
    }

    OGFX_LOG("Image transition:");

    const ImageStateSyncInfo src = get_image_state_src_info(info.old_state);
    const ImageStateSyncInfo dst = get_image_state_dst_info(info.new_state);

    ImageBarrierInfo barrier_info{};
    barrier_info.image = info.image;
    barrier_info.old_state = info.old_state;
    barrier_info.new_state = info.new_state;
    barrier_info.memory_barrier_info.src_stages = src.stages;
    barrier_info.memory_barrier_info.dst_stages = dst.stages;
    barrier_info.memory_barrier_info.src_access = src.access;
    barrier_info.memory_barrier_info.dst_access = dst.access;
    barrier_info.base_mip_level = info.base_mip_level;
    barrier_info.mip_level_count = info.mip_level_count;
    barrier_info.base_array_layer = info.base_array_layer;
    barrier_info.array_layer_count = info.array_layer_count;

    image_barrier(barrier_info);
}

} // namespace ogfx