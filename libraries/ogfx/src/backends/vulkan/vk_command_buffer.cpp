#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_command_buffer.hpp"
#include "vk_command_pool.hpp"
#include "vk_render_pass.hpp"
#include "vk_framebuffer.hpp"
#include "vk_graphics_pipeline.hpp"
#include "vk_buffer.hpp"
#include "vk_resource_set.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_image.hpp"
#include "vk_image_state.hpp"
#include "vk_format.hpp"
#include "vk_shader_stage.hpp"

#include <stdexcept>

namespace ogfx
{

CommandBuffer::CommandBuffer(const CommandPool& command_pool) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = command_pool.m_impl->m_device;
    m_impl->m_command_pool = command_pool.m_impl->m_command_pool;

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = m_impl->m_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    OGFX_LOG("Allocating Vulkan command buffer");

    if (vkAllocateCommandBuffers(m_impl->m_device, &allocate_info, &m_impl->m_command_buffer) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to allocate Vulkan command buffer");
        throw std::runtime_error("Failed to allocate Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer allocated");
}

CommandBuffer::~CommandBuffer()
{
    if (m_impl && m_impl->m_command_buffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_impl->m_device, m_impl->m_command_pool, 1, &m_impl->m_command_buffer);

        OGFX_LOG("Vulkan command buffer freed");
    }
}

CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;

CommandBuffer& CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

void CommandBuffer::begin()
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_impl->m_command_buffer, &begin_info) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to begin Vulkan command buffer");
        throw std::runtime_error("Failed to begin Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer recording started");
}

void CommandBuffer::end()
{
    if (vkEndCommandBuffer(m_impl->m_command_buffer) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to end Vulkan command buffer");
        throw std::runtime_error("Failed to end Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer recording ended");
}

void CommandBuffer::reset()
{
    if (vkResetCommandBuffer(m_impl->m_command_buffer, 0) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to reset Vulkan command buffer");
        throw std::runtime_error("Failed to reset Vulkan command buffer.");
    }

    OGFX_LOG("Vulkan command buffer reset");
}

void CommandBuffer::begin_render_pass(const RenderPass& render_pass, const Framebuffer& framebuffer,
                                      const RenderPassBeginInfo& begin_info)
{
    OGFX_LOG("Beginning Vulkan render pass");

    OGFX_LOG("  Render area:");
    OGFX_LOG("    Offset: (" + std::to_string(begin_info.offset_x) + ", " + std::to_string(begin_info.offset_y) + ")");
    OGFX_LOG("    Extent: " + std::to_string(begin_info.width) + "x" + std::to_string(begin_info.height));

    OGFX_LOG("  Clear color:");
    OGFX_LOG("    R: " + std::to_string(begin_info.clear_color.r));
    OGFX_LOG("    G: " + std::to_string(begin_info.clear_color.g));
    OGFX_LOG("    B: " + std::to_string(begin_info.clear_color.b));
    OGFX_LOG("    A: " + std::to_string(begin_info.clear_color.a));

    VkRenderPassBeginInfo vk_begin_info{};
    vk_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vk_begin_info.renderPass = render_pass.m_impl->m_render_pass;
    vk_begin_info.framebuffer = framebuffer.m_impl->m_framebuffer;

    vk_begin_info.renderArea.offset = {static_cast<int32_t>(begin_info.offset_x),
                                       static_cast<int32_t>(begin_info.offset_y)};

    vk_begin_info.renderArea.extent = {begin_info.width, begin_info.height};

    VkClearValue clear_value{};
    clear_value.color = {begin_info.clear_color.r, begin_info.clear_color.g, begin_info.clear_color.b,
                         begin_info.clear_color.a};

    vk_begin_info.clearValueCount = 1;
    vk_begin_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(m_impl->m_command_buffer, &vk_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    OGFX_LOG("Vulkan render pass begun");
}

void CommandBuffer::end_render_pass()
{
    vkCmdEndRenderPass(m_impl->m_command_buffer);

    OGFX_LOG("Vulkan render pass ended");
}

void CommandBuffer::bind_graphics_pipeline(const GraphicsPipeline& pipeline)
{
    OGFX_LOG("Binding Vulkan graphics pipeline");

    vkCmdBindPipeline(m_impl->m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_impl->m_pipeline);

    OGFX_LOG("Vulkan graphics pipeline bound");
}

void CommandBuffer::draw(const DrawInfo& draw_info)
{
    OGFX_LOG("Drawing:");
    OGFX_LOG("  Vertex count: " + std::to_string(draw_info.vertex_count));
    OGFX_LOG("  Instance count: " + std::to_string(draw_info.instance_count));
    OGFX_LOG("  First vertex: " + std::to_string(draw_info.first_vertex));
    OGFX_LOG("  First instance: " + std::to_string(draw_info.first_instance));

    vkCmdDraw(m_impl->m_command_buffer, draw_info.vertex_count, draw_info.instance_count, draw_info.first_vertex,
              draw_info.first_instance);

    OGFX_LOG("Vertices drawn");
}

void CommandBuffer::bind_vertex_buffer(const VertexBufferBindInfo& info)
{
    OGFX_LOG("Binding vertex buffer:");
    OGFX_LOG("  Binding: " + std::to_string(info.binding));
    OGFX_LOG("  Offset: " + std::to_string(info.offset));

    VkBuffer vertex_buffer = info.buffer->m_impl->m_buffer;
    VkDeviceSize buffer_offset = static_cast<VkDeviceSize>(info.offset);

    vkCmdBindVertexBuffers(m_impl->m_command_buffer, info.binding, 1, &vertex_buffer, &buffer_offset);

    OGFX_LOG("Vertex buffer bound");
}

void CommandBuffer::bind_resource_set(const ResourceSetBindInfo& info)
{
    OGFX_LOG("Binding resource set at index " + std::to_string(info.set_index));

    const VkDescriptorSet descriptor_set = info.resource_set->m_impl->m_descriptor_set;

    vkCmdBindDescriptorSets(m_impl->m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            info.layout->m_impl->m_pipeline_layout, info.set_index, 1, &descriptor_set, 0, nullptr);

    OGFX_LOG("Resource set bound");
}

void CommandBuffer::set_inline_constants(const InlineConstantsInfo& info) 
{
    OGFX_LOG("Setting inline constants:");
    OGFX_LOG("  Stages: " + to_string_bitmask(info.stages));
    OGFX_LOG("  Offset: " + std::to_string(info.offset));
    OGFX_LOG("  Size: " + std::to_string(info.size));

    vkCmdPushConstants(m_impl->m_command_buffer, info.layout->m_impl->m_pipeline_layout,
                       to_vk_shader_stage_flags(info.stages), info.offset, info.size, info.data);

    OGFX_LOG("Inline constants set");
}

void CommandBuffer::transition_image(const ImageTransitionInfo& info) 
{
    if (!m_impl || !info.image)
    {
        OGFX_LOG_ERROR("Cannot transition image: invalid transition info");
        return;
    }

    const VkImageLayout old_layout = to_vk_image_state(info.old_state);
    const VkImageLayout new_layout = to_vk_image_state(info.new_state);

    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkAccessFlags src_access = 0;
    VkAccessFlags dst_access = 0;

    switch (info.old_state)
    {
    case ImageState::Undefined:
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        src_access = 0;
        break;

    case ImageState::TransferDst:
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        src_access = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case ImageState::TransferSrc:
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        src_access = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case ImageState::ShaderRead:
        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        src_access = VK_ACCESS_SHADER_READ_BIT;
        break;

    case ImageState::RenderTarget:
        src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        src_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    default:
        src_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        src_access = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        break;
    }

    switch (info.new_state)
    {
    case ImageState::TransferDst:
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_access = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case ImageState::TransferSrc:
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_access = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case ImageState::ShaderRead:
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_access = VK_ACCESS_SHADER_READ_BIT;
        break;

    case ImageState::RenderTarget:
        dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dst_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case ImageState::Present:
        dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dst_access = 0;
        break;

    default:
        dst_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        dst_access = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        break;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = src_access;
    barrier.dstAccessMask = dst_access;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = info.image->m_impl->m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = info.base_mip_level;
    barrier.subresourceRange.levelCount = info.mip_level_count;
    barrier.subresourceRange.baseArrayLayer = info.base_array_layer;
    barrier.subresourceRange.layerCount = info.array_layer_count;

    vkCmdPipelineBarrier(m_impl->m_command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    OGFX_LOG("Image transition:");
    OGFX_LOG("  Old state: " + to_string_enum(info.old_state));
    OGFX_LOG("  New state: " + to_string_enum(info.new_state));
    OGFX_LOG("  Base mip level: " + std::to_string(info.base_mip_level));
    OGFX_LOG("  Mip level count: " + std::to_string(info.mip_level_count));
    OGFX_LOG("  Base array layer: " + std::to_string(info.base_array_layer));
    OGFX_LOG("  Array layer count: " + std::to_string(info.array_layer_count));
}

void CommandBuffer::copy_buffer_to_image(const BufferToImageCopyInfo& info)
{
    if (!m_impl || !info.buffer || !info.image || info.width == 0 || info.height == 0 || info.depth == 0 ||
        info.array_layer_count == 0)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to image: invalid copy info");
        return;
    }

    if (info.mip_level >= info.image->m_desc.mip_levels || info.base_array_layer >= info.image->m_desc.array_layers ||
        info.array_layer_count > info.image->m_desc.array_layers - info.base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to image: mip level or array layer range is out of bounds");
        return;
    }

    const uint32_t mip_width = std::max(1u, info.image->width() >> info.mip_level);
    const uint32_t mip_height = std::max(1u, info.image->height() >> info.mip_level);

    if (info.width > mip_width || info.height > mip_height || info.depth > 1)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to image: copy extent exceeds mip dimensions");
        return;
    }

    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = info.buffer_offset;
    copy_region.bufferRowLength = info.buffer_row_length;
    copy_region.bufferImageHeight = info.buffer_image_height;

    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = info.mip_level;
    copy_region.imageSubresource.baseArrayLayer = info.base_array_layer;
    copy_region.imageSubresource.layerCount = info.array_layer_count;

    copy_region.imageOffset = {0, 0, 0};
    copy_region.imageExtent = {info.width, info.height, info.depth};

    vkCmdCopyBufferToImage(m_impl->m_command_buffer, info.buffer->m_impl->m_buffer, info.image->m_impl->m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    OGFX_LOG("Recorded Vulkan buffer-to-image copy:");
    OGFX_LOG("  Buffer offset: " + std::to_string(info.buffer_offset));
    OGFX_LOG("  Buffer row length: " + std::to_string(info.buffer_row_length));
    OGFX_LOG("  Buffer image height: " + std::to_string(info.buffer_image_height));
    OGFX_LOG("  Mip level: " + std::to_string(info.mip_level));
    OGFX_LOG("  Base array layer: " + std::to_string(info.base_array_layer));
    OGFX_LOG("  Array layer count: " + std::to_string(info.array_layer_count));
    OGFX_LOG("  Extent: " + std::to_string(info.width) + "x" + std::to_string(info.height) + "x" + std::to_string(info.depth));
}

void CommandBuffer::generate_mipmaps(const GenerateMipmapsInfo& info)
{
    if (!m_impl || !info.image)
    {
        OGFX_LOG_ERROR("Cannot generate mipmaps: invalid generate info");
        return;
    }

    const Image& image = *info.image;
    const uint32_t mip_levels = image.m_desc.mip_levels;

    if (mip_levels <= 1)
    {
        OGFX_LOG("Skipping mipmap generation: image has only one mip level");
        return;
    }

    const VkImage vk_image = image.m_impl->m_image;
    const VkFormat format = to_vk_format(image.format());

    VkFormatProperties format_properties{};
    vkGetPhysicalDeviceFormatProperties(image.m_impl->m_physical_device, format, &format_properties);

    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) == 0 ||
        (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) == 0 ||
        (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
    {
        OGFX_LOG_ERROR("Cannot generate mipmaps: image format does not support linear blitting");
        return;
    }

    uint32_t mip_width = image.width();
    uint32_t mip_height = image.height();

    OGFX_LOG("Generating Vulkan image mipmaps:");
    OGFX_LOG("  Format: " + to_string_enum(image.format()));
    OGFX_LOG("  Base extent: " + std::to_string(mip_width) + "x" + std::to_string(mip_height));
    OGFX_LOG("  Mip levels: " + std::to_string(mip_levels));

    for (uint32_t level = 1; level < mip_levels; ++level)
    {
        // The previous level was written as a transfer destination.
        ImageTransitionInfo source_transition{};
        source_transition.image = &image;
        source_transition.old_state = ImageState::TransferDst;
        source_transition.new_state = ImageState::TransferSrc;
        source_transition.base_mip_level = level - 1;
        source_transition.mip_level_count = 1;
        source_transition.base_array_layer = 0;
        source_transition.array_layer_count = image.m_desc.array_layers;

        transition_image(source_transition);

        // Prepare the next mip level to receive the blit.
        ImageTransitionInfo destination_transition{};
        destination_transition.image = &image;
        destination_transition.old_state = ImageState::Undefined;
        destination_transition.new_state = ImageState::TransferDst;
        destination_transition.base_mip_level = level;
        destination_transition.mip_level_count = 1;
        destination_transition.base_array_layer = 0;
        destination_transition.array_layer_count = image.m_desc.array_layers;

        transition_image(destination_transition);

        const uint32_t next_width = std::max(1u, mip_width / 2);
        const uint32_t next_height = std::max(1u, mip_height / 2);

        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = level - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = image.m_desc.array_layers;
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_height), 1};

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = level;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = image.m_desc.array_layers;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {static_cast<int32_t>(next_width), static_cast<int32_t>(next_height), 1};

        vkCmdBlitImage(m_impl->m_command_buffer, vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        OGFX_LOG("  Generated mip " + std::to_string(level) + ": " + std::to_string(next_width) + "x" +
                 std::to_string(next_height));

        // The current source level can now become the next blit's source.
        mip_width = next_width;
        mip_height = next_height;
    }

    // The final mip level is still a transfer destination.
    ImageTransitionInfo last_transition{};
    last_transition.image = &image;
    last_transition.old_state = ImageState::TransferDst;
    last_transition.new_state = ImageState::ShaderRead;
    last_transition.base_mip_level = mip_levels - 1;
    last_transition.mip_level_count = 1;
    last_transition.base_array_layer = 0;
    last_transition.array_layer_count = image.m_desc.array_layers;

    transition_image(last_transition);

    // All earlier levels are transfer sources.
    ImageTransitionInfo source_levels_transition{};
    source_levels_transition.image = &image;
    source_levels_transition.old_state = ImageState::TransferSrc;
    source_levels_transition.new_state = ImageState::ShaderRead;
    source_levels_transition.base_mip_level = 0;
    source_levels_transition.mip_level_count = mip_levels - 1;
    source_levels_transition.base_array_layer = 0;
    source_levels_transition.array_layer_count = image.m_desc.array_layers;

    transition_image(source_levels_transition);

    OGFX_LOG("Mipmap generation recorded");
}

} // namespace ogfx