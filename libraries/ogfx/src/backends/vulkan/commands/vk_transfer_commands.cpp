#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/transfer_commands.hpp>
#include <ogfx/commands/barrier_commands.hpp>

#include <vulkan/vulkan.h>

#include "../vk_command_buffer.hpp"
#include "../vk_format.hpp"
#include "../vk_buffer.hpp"
#include "../vk_image.hpp"
#include "../vk_image_aspect.hpp"

namespace ogfx
{

void CommandBuffer::copy_buffer_to_buffer(const BufferToBufferCopyInfo& info)
{
    if (!m_impl || !info.src_buffer || !info.dst_buffer || info.size == 0)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to buffer: invalid copy info");
        return;
    }

    const uint64_t src_size = info.src_buffer->m_desc.size;
    const uint64_t dst_size = info.dst_buffer->m_desc.size;

    if (info.src_offset > src_size || info.size > src_size - info.src_offset || info.dst_offset > dst_size ||
        info.size > dst_size - info.dst_offset)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to buffer: copy range is out of bounds");
        return;
    }

    if (info.src_buffer == info.dst_buffer)
    {
        const uint64_t src_end = info.src_offset + info.size;
        const uint64_t dst_end = info.dst_offset + info.size;

        if (info.src_offset < dst_end && info.dst_offset < src_end)
        {
            OGFX_LOG_ERROR("Cannot copy buffer to buffer: source and destination ranges overlap");
            return;
        }
    }

    VkBufferCopy copy_region{};
    copy_region.srcOffset = info.src_offset;
    copy_region.dstOffset = info.dst_offset;
    copy_region.size = info.size;

    vkCmdCopyBuffer(m_impl->m_command_buffer, info.src_buffer->m_impl->m_buffer, info.dst_buffer->m_impl->m_buffer, 1,
                    &copy_region);

    OGFX_LOG("Recorded Vulkan buffer-to-buffer copy:");
    OGFX_LOG("  Source offset: " + std::to_string(info.src_offset));
    OGFX_LOG("  Destination offset: " + std::to_string(info.dst_offset));
    OGFX_LOG("  Size: " + std::to_string(info.size));
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
    const uint32_t mip_depth = std::max(1u, info.image->depth() >> info.mip_level);

    if (info.width > mip_width || info.height > mip_height || info.depth > mip_depth)
    {
        OGFX_LOG_ERROR("Cannot copy buffer to image: copy extent exceeds mip dimensions");
        return;
    }

    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = info.buffer_offset;
    copy_region.bufferRowLength = info.buffer_row_length;
    copy_region.bufferImageHeight = info.buffer_image_height;

    copy_region.imageSubresource.aspectMask = to_vk_image_aspect(info.aspect);
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
    OGFX_LOG("  Aspect: " + to_string_bitmask(info.aspect));
}

void CommandBuffer::copy_image_to_image(const ImageToImageCopyInfo& info)
{
    if (!m_impl || !info.src_image || !info.dst_image || info.width == 0 || info.height == 0 || info.depth == 0 ||
        info.array_layer_count == 0)
    {
        OGFX_LOG_ERROR("Cannot copy image to image: invalid copy info parameters");
        return;
    }

    if (info.src_mip_level >= info.src_image->m_desc.mip_levels ||
        info.src_base_array_layer >= info.src_image->m_desc.array_layers ||
        info.array_layer_count > info.src_image->m_desc.array_layers - info.src_base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot copy image to image: source mip level or array layer range out of bounds");
        return;
    }

    if (info.dst_mip_level >= info.dst_image->m_desc.mip_levels ||
        info.dst_base_array_layer >= info.dst_image->m_desc.array_layers ||
        info.array_layer_count > info.dst_image->m_desc.array_layers - info.dst_base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot copy image to image: destination mip level or array layer range out of bounds");
        return;
    }

    const uint32_t src_mip_width = std::max(1u, info.src_image->width() >> info.src_mip_level);
    const uint32_t src_mip_height = std::max(1u, info.src_image->height() >> info.src_mip_level);
    const uint32_t src_mip_depth = std::max(1u, info.src_image->depth() >> info.src_mip_level);
    const uint32_t dst_mip_width = std::max(1u, info.dst_image->width() >> info.dst_mip_level);
    const uint32_t dst_mip_height = std::max(1u, info.dst_image->height() >> info.dst_mip_level);
    const uint32_t dst_mip_depth = std::max(1u, info.dst_image->depth() >> info.dst_mip_level);

    if (info.width > src_mip_width || info.height > src_mip_height || info.depth > src_mip_depth ||
        info.width > dst_mip_width || info.height > dst_mip_height || info.depth > dst_mip_depth)
    {
        OGFX_LOG_ERROR(
            "Cannot copy image to image: copy extent exceeds source or destination subresource layout dimensions");
        return;
    }

    VkImageCopy copy_region{};

    copy_region.srcSubresource.aspectMask = to_vk_image_aspect(info.src_aspect);
    copy_region.srcSubresource.mipLevel = info.src_mip_level;
    copy_region.srcSubresource.baseArrayLayer = info.src_base_array_layer;
    copy_region.srcSubresource.layerCount = info.array_layer_count;
    copy_region.srcOffset = {0, 0, 0};

    copy_region.dstSubresource.aspectMask = to_vk_image_aspect(info.dst_aspect);
    copy_region.dstSubresource.mipLevel = info.dst_mip_level;
    copy_region.dstSubresource.baseArrayLayer = info.dst_base_array_layer;
    copy_region.dstSubresource.layerCount = info.array_layer_count;
    copy_region.dstOffset = {0, 0, 0};

    copy_region.extent = {info.width, info.height, info.depth};

    vkCmdCopyImage(m_impl->m_command_buffer, info.src_image->m_impl->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   info.dst_image->m_impl->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    OGFX_LOG("Recorded Vulkan image-to-image copy:");
    OGFX_LOG("  Src Mip level: " + std::to_string(info.src_mip_level) +
             ", Src Base Layer: " + std::to_string(info.src_base_array_layer));
    OGFX_LOG("  Src Aspect: " + to_string_bitmask(info.src_aspect));
    OGFX_LOG("  Dst Mip level: " + std::to_string(info.dst_mip_level) +
             ", Dst Base Layer: " + std::to_string(info.dst_base_array_layer));
    OGFX_LOG("  Dst Aspect: " + to_string_bitmask(info.dst_aspect));
    OGFX_LOG("  Layer count:   " + std::to_string(info.array_layer_count));
    OGFX_LOG("  Extent:        " + std::to_string(info.width) + "x" + std::to_string(info.height) + "x" + std::to_string(info.depth));
}

void CommandBuffer::blit_image(const ImageBlitInfo& info)
{
    if (!m_impl || !info.src_image || !info.dst_image || info.src_width == 0 || info.src_height == 0 ||
        info.dst_width == 0 || info.dst_height == 0 || info.depth == 0 || info.array_layer_count == 0)
    {
        OGFX_LOG_ERROR("Cannot blit image: invalid blit info");
        return;
    }

    if (info.src_mip_level >= info.src_image->m_desc.mip_levels ||
        info.src_base_array_layer >= info.src_image->m_desc.array_layers ||
        info.array_layer_count > info.src_image->m_desc.array_layers - info.src_base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot blit image: source mip level or array layer range is out of bounds");
        return;
    }

    const auto& src = *info.src_image;
    const auto& dst = *info.dst_image;

    if (info.src_mip_level >= src.m_desc.mip_levels || info.dst_mip_level >= dst.m_desc.mip_levels ||
        info.src_base_array_layer >= src.m_desc.array_layers || info.dst_base_array_layer >= dst.m_desc.array_layers ||
        info.array_layer_count > src.m_desc.array_layers - info.src_base_array_layer ||
        info.array_layer_count > dst.m_desc.array_layers - info.dst_base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot blit image: invalid mip or array layer range");
        return;
    }

    const uint32_t src_width = std::max(1u, src.width() >> info.src_mip_level);
    const uint32_t src_height = std::max(1u, src.height() >> info.src_mip_level);
    const uint32_t src_depth = std::max(1u, src.depth() >> info.src_mip_level);
    const uint32_t dst_width = std::max(1u, dst.width() >> info.dst_mip_level);
    const uint32_t dst_height = std::max(1u, dst.height() >> info.dst_mip_level);
    const uint32_t dst_depth = std::max(1u, dst.depth() >> info.dst_mip_level);

    // Validate offsets and extents
    auto within_bounds = [](int32_t offset, uint32_t extent, uint32_t limit)
    { return offset >= 0 && static_cast<uint64_t>(offset) + extent <= limit; };

    if (!within_bounds(info.src_x, info.src_width, src_width) ||
        !within_bounds(info.src_y, info.src_height, src_height) ||
        !within_bounds(info.dst_x, info.dst_width, dst_width) ||
        !within_bounds(info.dst_y, info.dst_height, dst_height) || info.depth > src_depth || info.depth > dst_depth)
    {
        OGFX_LOG_ERROR("Cannot blit image: source or destination region out of bounds");
        return;
    }

    VkImageBlit region{};

    region.srcSubresource.aspectMask = to_vk_image_aspect(info.src_aspect);
    region.srcSubresource.mipLevel = info.src_mip_level;
    region.srcSubresource.baseArrayLayer = info.src_base_array_layer;
    region.srcSubresource.layerCount = info.array_layer_count;
    region.srcOffsets[0] = {info.src_x, info.src_y, 0};
    region.srcOffsets[1] = {info.src_x + static_cast<int32_t>(info.src_width),
                            info.src_y + static_cast<int32_t>(info.src_height), static_cast<int32_t>(info.depth)};

    region.dstSubresource.aspectMask = to_vk_image_aspect(info.dst_aspect);
    region.dstSubresource.mipLevel = info.dst_mip_level;
    region.dstSubresource.baseArrayLayer = info.dst_base_array_layer;
    region.dstSubresource.layerCount = info.array_layer_count;
    region.dstOffsets[0] = {info.dst_x, info.dst_y, 0};
    region.dstOffsets[1] = {info.dst_x + static_cast<int32_t>(info.dst_width),
                            info.dst_y + static_cast<int32_t>(info.dst_height), static_cast<int32_t>(info.depth)};

    vkCmdBlitImage(m_impl->m_command_buffer, info.src_image->m_impl->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   info.dst_image->m_impl->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region,
                   VK_FILTER_NEAREST);

    OGFX_LOG("Recorded Vulkan image-to-image blit:");
    
    OGFX_LOG("  Source:");
    OGFX_LOG("    Mip level: " + std::to_string(info.src_mip_level));
    OGFX_LOG("    Base array layer: " + std::to_string(info.src_base_array_layer));
    OGFX_LOG("    Aspect: " + to_string_bitmask(info.src_aspect));
    OGFX_LOG("    Offset: (" + std::to_string(info.src_x) + ", " + std::to_string(info.src_y) + ")");
    OGFX_LOG("    Extent: " + std::to_string(info.src_width) + "x" + std::to_string(info.src_height));
    
    OGFX_LOG("  Destination:");
    OGFX_LOG("    Mip level: " + std::to_string(info.dst_mip_level));
    OGFX_LOG("    Base array layer: " + std::to_string(info.dst_base_array_layer));
    OGFX_LOG("    Aspect: " + to_string_bitmask(info.dst_aspect));
    OGFX_LOG("    Offset: (" + std::to_string(info.dst_x) + ", " + std::to_string(info.dst_y) + ")");
    OGFX_LOG("    Extent: " + std::to_string(info.dst_width) + "x" + std::to_string(info.dst_height));
    
    OGFX_LOG("  Depth: " + std::to_string(info.depth));
    OGFX_LOG("  Array layer count: " + std::to_string(info.array_layer_count));
}

void CommandBuffer::clear_image(const ImageClearInfo& info)
{
    if (!m_impl || !info.image)
    {
        OGFX_LOG_ERROR("Cannot clear image: invalid info");
        return;
    }

    const auto& desc = info.image->m_desc;

    if (info.mip_level_count == 0 || info.array_layer_count == 0 || info.base_mip_level >= desc.mip_levels ||
        info.base_array_layer >= desc.array_layers || info.mip_level_count > desc.mip_levels - info.base_mip_level ||
        info.array_layer_count > desc.array_layers - info.base_array_layer)
    {
        OGFX_LOG_ERROR("Cannot clear image: subresource range out of bounds");
        return;
    }

    VkClearColorValue color{};
    color.float32[0] = info.color[0];
    color.float32[1] = info.color[1];
    color.float32[2] = info.color[2];
    color.float32[3] = info.color[3];

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = info.base_mip_level;
    range.levelCount = info.mip_level_count;
    range.baseArrayLayer = info.base_array_layer;
    range.layerCount = info.array_layer_count;

    vkCmdClearColorImage(m_impl->m_command_buffer, info.image->m_impl->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         &color, 1, &range);

    OGFX_LOG("Recorded Vulkan image clear:");
    OGFX_LOG("  Image extent: " + std::to_string(info.image->width()) + "x" + std::to_string(info.image->height()) + "x" + std::to_string(info.image->depth()));
    OGFX_LOG("  Color: (" + std::to_string(info.color[0]) + ", " + std::to_string(info.color[1]) + ", " + std::to_string(info.color[2]) + ", " + std::to_string(info.color[3]) + ")");
    OGFX_LOG("  Base mip level: " + std::to_string(info.base_mip_level));
    OGFX_LOG("  Mip level count: " + std::to_string(info.mip_level_count));
    OGFX_LOG("  Base array layer: " + std::to_string(info.base_array_layer));
    OGFX_LOG("  Array layer count: " + std::to_string(info.array_layer_count));
    OGFX_LOG("  Aspect: Color");
    OGFX_LOG("  Image state: TransferDst");
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
        // The previous level was written as a transfer destination
        ImageTransitionInfo source_transition{};
        source_transition.image = &image;
        source_transition.old_state = ImageState::TransferDst;
        source_transition.new_state = ImageState::TransferSrc;
        source_transition.base_mip_level = level - 1;
        source_transition.mip_level_count = 1;
        source_transition.base_array_layer = 0;
        source_transition.array_layer_count = image.m_desc.array_layers;

        transition_image(source_transition);

        // Prepare the next mip level to receive the blit
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

        // The current source level can now become the next blit's source
        mip_width = next_width;
        mip_height = next_height;
    }

    // The final mip level is still a transfer destination
    ImageTransitionInfo last_transition{};
    last_transition.image = &image;
    last_transition.old_state = ImageState::TransferDst;
    last_transition.new_state = ImageState::ShaderRead;
    last_transition.base_mip_level = mip_levels - 1;
    last_transition.mip_level_count = 1;
    last_transition.base_array_layer = 0;
    last_transition.array_layer_count = image.m_desc.array_layers;

    transition_image(last_transition);

    // All earlier levels are transfer sources
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