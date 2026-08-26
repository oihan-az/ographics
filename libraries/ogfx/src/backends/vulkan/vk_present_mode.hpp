#ifndef OGFX_VK_PRESENT_MODE_HPP
#define OGFX_VK_PRESENT_MODE_HPP

#include <ogfx/present_mode.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkPresentModeKHR to_vk_present_mode(PresentMode mode);
PresentMode from_vk_present_mode(VkPresentModeKHR mode);

} // namespace ogfx

#endif // OGFX_VK_PRESENT_MODE_HPP