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

namespace ogfx
{

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
    OGFX_LOG("  Extent: " + std::to_string(info.width) + "x" + std::to_string(info.height) + "x" +
             std::to_string(info.depth));
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