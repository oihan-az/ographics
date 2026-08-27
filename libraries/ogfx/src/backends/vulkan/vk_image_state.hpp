#ifndef OGFX_VK_IMAGE_STATE_HPP
#define OGFX_VK_IMAGE_STATE_HPP

#include <ogfx/image_state.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkImageLayout to_vk_image_state(ImageState state);
ImageState from_vk_image_state(VkImageLayout layout);

} // namespace ogfx

#endif // OGFX_VK_IMAGE_STATE_HPP