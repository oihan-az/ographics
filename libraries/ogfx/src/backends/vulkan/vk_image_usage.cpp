#include "vk_image_usage.hpp"

namespace ogfx
{

VkImageUsageFlags to_vk_image_usage(ImageUsage usage)
{
    VkImageUsageFlags flags = 0;

    if (has_flag(usage, ImageUsage::TransferSrc))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (has_flag(usage, ImageUsage::TransferDst))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (has_flag(usage, ImageUsage::Sampled))
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (has_flag(usage, ImageUsage::Storage))
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (has_flag(usage, ImageUsage::ColorAttachment))
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (has_flag(usage, ImageUsage::DepthStencil))
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return flags;
}

ImageUsage from_vk_image_usage(VkImageUsageFlags flags)
{
    ImageUsage usage = ImageUsage::None;

    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        usage = usage | ImageUsage::TransferSrc;
    }

    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        usage = usage | ImageUsage::TransferDst;
    }

    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT)
    {
        usage = usage | ImageUsage::Sampled;
    }

    if (flags & VK_IMAGE_USAGE_STORAGE_BIT)
    {
        usage = usage | ImageUsage::Storage;
    }

    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        usage = usage | ImageUsage::ColorAttachment;
    }

    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        usage = usage | ImageUsage::DepthStencil;
    }

    return usage;
}

} // namespace ogfx