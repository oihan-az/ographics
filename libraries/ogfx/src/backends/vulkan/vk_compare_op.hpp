#ifndef OGFX_VK_COMPARE_OP_HPP
#define OGFX_VK_COMPARE_OP_HPP

#include <ogfx/compare_op.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkCompareOp to_vk_compare_op(CompareOp op);
CompareOp from_vk_compare_op(VkCompareOp op);

} // namespace ogfx

#endif // OGFX_VK_COMPARE_OP_HPP