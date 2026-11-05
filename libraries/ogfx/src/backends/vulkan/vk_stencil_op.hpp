#ifndef OGFX_VK_STENCIL_OP_HPP
#define OGFX_VK_STENCIL_OP_HPP

#include <ogfx/stencil_op.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkStencilOp to_vk_stencil_op(StencilOp op);
StencilOp from_vk_stencil_op(VkStencilOp op);

} // namespace ogfx

#endif // OGFX_VK_STENCIL_OP_HPP