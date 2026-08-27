#include "vk_image_state.hpp"

namespace ogfx
{

VkImageLayout to_vk_image_state(ImageState state)
{
    switch (state)
    {
    case ImageState::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;

    case ImageState::Common:
        return VK_IMAGE_LAYOUT_GENERAL;

    case ImageState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case ImageState::DepthStencil:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case ImageState::ShaderRead:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case ImageState::TransferSrc:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    case ImageState::TransferDst:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    case ImageState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

ImageState from_vk_image_state(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        return ImageState::Undefined;

    case VK_IMAGE_LAYOUT_GENERAL:
        return ImageState::Common;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return ImageState::RenderTarget;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return ImageState::DepthStencil;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return ImageState::ShaderRead;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return ImageState::TransferSrc;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return ImageState::TransferDst;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        return ImageState::Present;

    default:
        return ImageState::Common;
    }
}

} // namespace ogfx