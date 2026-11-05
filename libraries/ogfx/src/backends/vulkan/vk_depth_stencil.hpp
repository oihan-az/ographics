#ifndef OGFX_VK_DEPTH_STENCIL_HPP
#define OGFX_VK_DEPTH_STENCIL_HPP

#include <ogfx/depth_stencil.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkStencilOpState to_vk_stencil_op_state(const StencilOpState& state);
StencilOpState from_vk_stencil_op_state(const VkStencilOpState& state);

} // namespace ogfx

#endif // OGFX_VK_DEPTH_STENCIL_HPP