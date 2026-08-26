#ifndef OGFX_VK_FORMAT_HPP
#define OGFX_VK_FORMAT_HPP

#include <ogfx/format.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkFormat to_vk_format(Format format);
Format from_vk_format(VkFormat format);

} // namespace ogfx

#endif // OGFX_VK_FORMAT_HPP