#ifndef OGFX_VK_LOAD_STORE_OP_HPP
#define OGFX_VK_LOAD_STORE_OP_HPP

#include <ogfx/load_store_op.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkAttachmentLoadOp to_vk_load_op(LoadOp op);
LoadOp from_vk_load_op(VkAttachmentLoadOp op);

VkAttachmentStoreOp to_vk_store_op(StoreOp op);
StoreOp from_vk_store_op(VkAttachmentStoreOp op);

} // namespace ogfx

#endif // OGFX_VK_LOAD_STORE_OP_HPP