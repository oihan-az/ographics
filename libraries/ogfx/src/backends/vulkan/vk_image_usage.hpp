#ifndef OGFX_VK_IMAGE_USAGE_HPP
#define OGFX_VK_IMAGE_USAGE_HPP

#include <ogfx/image_usage.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkImageUsageFlags to_vk_image_usage(ImageUsage usage);
ImageUsage from_vk_image_usage(VkImageUsageFlags flags);

} // namespace ogfx

#endif // OGFX_VK_IMAGE_USAGE_HPP