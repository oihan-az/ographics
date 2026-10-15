#ifndef OGFX_VK_COLOR_BLEND_HPP
#define OGFX_VK_COLOR_BLEND_HPP

#include <ogfx/color_blend.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkBlendFactor to_vk_blend_factor(BlendFactor factor);
BlendFactor from_vk_blend_factor(VkBlendFactor factor);

VkBlendOp to_vk_blend_op(BlendOp op);
BlendOp from_vk_blend_op(VkBlendOp op);

VkColorComponentFlags to_vk_color_components(ColorComponent components);
ColorComponent from_vk_color_components(VkColorComponentFlags components);

} // namespace ogfx

#endif // OGFX_VK_COLOR_BLEND_HPP