#ifndef OGFX_VK_IMAGE_ASPECT_HPP
#define OGFX_VK_IMAGE_ASPECT_HPP

#include <ogfx/image_aspect.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkImageAspectFlags to_vk_image_aspect(ImageAspect aspect);
ImageAspect from_vk_image_aspect(VkImageAspectFlags flags);

} // namespace ogfx

#endif // OGFX_VK_IMAGE_ASPECT_HPP