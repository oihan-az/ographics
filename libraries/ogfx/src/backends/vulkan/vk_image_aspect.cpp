#include "vk_image_aspect.hpp"

namespace ogfx
{

VkImageAspectFlags to_vk_image_aspect(ImageAspect aspect)
{
    VkImageAspectFlags flags = 0;

    if (has_flag(aspect, ImageAspect::Color))
    {
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (has_flag(aspect, ImageAspect::Depth))
    {
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (has_flag(aspect, ImageAspect::Stencil))
    {
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return flags;
}

ImageAspect from_vk_image_aspect(VkImageAspectFlags flags)
{
    ImageAspect aspect = ImageAspect::None;

    if (flags & VK_IMAGE_ASPECT_COLOR_BIT)
    {
        aspect = aspect | ImageAspect::Color;
    }

    if (flags & VK_IMAGE_ASPECT_DEPTH_BIT)
    {
        aspect = aspect | ImageAspect::Depth;
    }

    if (flags & VK_IMAGE_ASPECT_STENCIL_BIT)
    {
        aspect = aspect | ImageAspect::Stencil;
    }

    return aspect;
}

} // namespace ogfx