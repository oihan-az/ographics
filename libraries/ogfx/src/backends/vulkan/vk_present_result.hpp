#ifndef OGFX_VK_PRESENT_RESULT_HPP
#define OGFX_VK_PRESENT_RESULT_HPP

#include <ogfx/present_result.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkResult to_vk_present_result(PresentResult result);
PresentResult from_vk_present_result(VkResult result);

} // namespace ogfx

#endif // OGFX_VK_PRESENT_RESULT_HPP